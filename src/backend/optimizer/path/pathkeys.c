/*-------------------------------------------------------------------------
 *
 * pathkeys.c
 *	  Utilities for matching and building path keys
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/optimizer/path/pathkeys.c,v 1.17 2000/01/09 00:26:33 tgl Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "nodes/makefuncs.h"
#include "optimizer/clauses.h"
#include "optimizer/joininfo.h"
#include "optimizer/paths.h"
#include "optimizer/tlist.h"
#include "optimizer/var.h"
#include "parser/parsetree.h"
#include "parser/parse_func.h"
#include "utils/lsyscache.h"

static PathKeyItem *makePathKeyItem(Node *key, Oid sortop);
static Var *find_indexkey_var(int indexkey, List *tlist);
static List *build_join_pathkey(List *pathkeys, List *join_rel_tlist,
								List *joinclauses);


/*--------------------
 *	Explanation of Path.pathkeys
 *
 *	Path.pathkeys is a List of Lists of PathKeyItem nodes that represent
 *	the sort order of the result generated by the Path.  The n'th sublist
 *	represents the n'th sort key of the result.
 *
 *	In single/base relation RelOptInfo's, the Paths represent various ways
 *	of scanning the relation and the resulting ordering of the tuples.
 *	Sequential scan Paths have NIL pathkeys, indicating no known ordering.
 *	Index scans have Path.pathkeys that represent the chosen index's ordering,
 *  if any.  A single-key index would create a pathkey with a single sublist,
 *	e.g. ( (tab1.indexkey1/sortop1) ).  A multi-key index generates a sublist
 *	per key, e.g. ( (tab1.indexkey1/sortop1) (tab1.indexkey2/sortop2) ) which
 *	shows major sort by indexkey1 (ordering by sortop1) and minor sort by
 *	indexkey2 with sortop2.
 *
 *	Note that a multi-pass indexscan (OR clause scan) has NIL pathkeys since
 *	we can say nothing about the overall order of its result.  Also, an
 *	indexscan on an unordered type of index generates NIL pathkeys.  However,
 *	we can always create a pathkey by doing an explicit sort.
 *
 *	Multi-relation RelOptInfo Path's are more complicated.  Mergejoins are
 *	only performed with equijoins ("=").  Because of this, the resulting
 *	multi-relation path actually has more than one primary key.  For example,
 *	a mergejoin using a clause "tab1.col1 = tab2.col1" would generate pathkeys
 *	of ( (tab1.col1/sortop1 tab2.col1/sortop2) ), indicating that the major
 *	sort order of the Path can be taken to be *either* tab1.col1 or tab2.col1.
 *	They are equal, so they are both primary sort keys.  This allows future
 *	joins to use either var as a pre-sorted key to prevent upper Mergejoins
 *	from having to re-sort the Path.  This is why pathkeys is a List of Lists.
 *
 *	Note that while the order of the top list is meaningful (primary vs.
 *	secondary sort key), the order of each sublist is arbitrary.  No code
 *	working with pathkeys should generate a result that depends on the order
 *	of a pathkey sublist.
 *
 *	We keep a sortop associated with each PathKeyItem because cross-data-type
 *	mergejoins are possible; for example int4=int8 is mergejoinable.  In this
 *	case we need to remember that the left var is ordered by int4lt while
 *	the right var is ordered by int8lt.  So the different members of each
 *	sublist could have different sortops.
 *
 *	When producing the pathkeys for a merge or nestloop join, we can keep
 *	all of the keys of the outer path, since the ordering of the outer path
 *	will be preserved in the result.  We add to each pathkey sublist any inner
 *	vars that are equijoined to any of the outer vars in the sublist.  In the
 *	nestloop case we have to be careful to consider only equijoin operators;
 *	the nestloop's join clauses might include non-equijoin operators.
 *	(Currently, we do this by considering only mergejoinable operators while
 *	making the pathkeys, since we have no separate marking for operators that
 *	are equijoins but aren't mergejoinable.)
 *
 *	Although Hashjoins also work only with equijoin operators, it is *not*
 *	safe to consider the output of a Hashjoin to be sorted in any particular
 *	order --- not even the outer path's order.  This is true because the
 *	executor might have to split the join into multiple batches.  Therefore
 *	a Hashjoin is always given NIL pathkeys.
 *
 *	Pathkeys are also useful to represent an ordering that we wish to achieve,
 *	since they are easily compared to the pathkeys of a potential candidate
 *	path.  So, SortClause lists are turned into pathkeys lists for use inside
 *	the optimizer.
 *
 *	-- bjm & tgl
 *--------------------
 */


/*
 * makePathKeyItem
 *		create a PathKeyItem node
 */
static PathKeyItem *
makePathKeyItem(Node *key, Oid sortop)
{
	PathKeyItem	   *item = makeNode(PathKeyItem);

	item->key = key;
	item->sortop = sortop;
	return item;
}

/****************************************************************************
 *		PATHKEY COMPARISONS
 ****************************************************************************/

/*
 * compare_pathkeys
 *	  Compare two pathkeys to see if they are equivalent, and if not whether
 *	  one is "better" than the other.
 *
 *	  A pathkey can be considered better than another if it is a superset:
 *	  it contains all the keys of the other plus more.  For example, either
 *	  ((A) (B)) or ((A B)) is better than ((A)).
 *
 *	This gets called a lot, so it is optimized.
 */
PathKeysComparison
compare_pathkeys(List *keys1, List *keys2)
{
	List	   *key1,
			   *key2;
	bool		key1_subsetof_key2 = true,
				key2_subsetof_key1 = true;

	for (key1 = keys1, key2 = keys2;
		 key1 != NIL && key2 != NIL;
		 key1 = lnext(key1), key2 = lnext(key2))
	{
		List	   *subkey1 = lfirst(key1);
		List	   *subkey2 = lfirst(key2);
		List	   *i;

		/* We have to do this the hard way since the ordering of the subkey
		 * lists is arbitrary.
		 */
		if (key1_subsetof_key2)
		{
			foreach(i, subkey1)
			{
				if (! member(lfirst(i), subkey2))
				{
					key1_subsetof_key2 = false;
					break;
				}
			}
		}

		if (key2_subsetof_key1)
		{
			foreach(i, subkey2)
			{
				if (! member(lfirst(i), subkey1))
				{
					key2_subsetof_key1 = false;
					break;
				}
			}
		}

		if (!key1_subsetof_key2 && !key2_subsetof_key1)
			return PATHKEYS_DIFFERENT; /* no need to keep looking */
	}

	/* If we reached the end of only one list, the other is longer and
	 * therefore not a subset.  (We assume the additional sublist(s)
	 * of the other list are not NIL --- no pathkey list should ever have
	 * a NIL sublist.)
	 */
	if (key1 != NIL)
		key1_subsetof_key2 = false;
	if (key2 != NIL)
		key2_subsetof_key1 = false;

	if (key1_subsetof_key2 && key2_subsetof_key1)
		return PATHKEYS_EQUAL;
	if (key1_subsetof_key2)
		return PATHKEYS_BETTER2;
	if (key2_subsetof_key1)
		return PATHKEYS_BETTER1;
	return PATHKEYS_DIFFERENT;
}

/*
 * pathkeys_contained_in
 *	  Common special case of compare_pathkeys: we just want to know
 *	  if keys2 are at least as well sorted as keys1.
 */
bool
pathkeys_contained_in(List *keys1, List *keys2)
{
	switch (compare_pathkeys(keys1, keys2))
	{
		case PATHKEYS_EQUAL:
		case PATHKEYS_BETTER2:
			return true;
		default:
			break;
	}
	return false;
}

/*
 * get_cheapest_path_for_pathkeys
 *	  Find the cheapest path in 'paths' that satisfies the given pathkeys.
 *	  Return NULL if no such path.
 *
 * 'paths' is a list of possible paths (either inner or outer)
 * 'pathkeys' represents a required ordering
 * if 'indexpaths_only' is true, only IndexPaths will be considered.
 */
Path *
get_cheapest_path_for_pathkeys(List *paths, List *pathkeys,
							   bool indexpaths_only)
{
	Path	   *matched_path = NULL;
	List	   *i;

	foreach(i, paths)
	{
		Path	   *path = (Path *) lfirst(i);

		if (indexpaths_only && ! IsA(path, IndexPath))
			continue;

		if (pathkeys_contained_in(pathkeys, path->pathkeys))
		{
			if (matched_path == NULL ||
				path->path_cost < matched_path->path_cost)
				matched_path = path;
		}
	}
	return matched_path;
}

/****************************************************************************
 *		NEW PATHKEY FORMATION
 ****************************************************************************/

/*
 * build_index_pathkeys
 *	  Build a pathkeys list that describes the ordering induced by an index
 *	  scan using the given index.  (Note that an unordered index doesn't
 *	  induce any ordering; such an index will have no sortop OIDS in
 *	  its "ordering" field.)
 *
 * Vars in the resulting pathkeys list are taken from the rel's targetlist.
 * If we can't find the indexkey in the targetlist, we assume that the
 * ordering of that key is not interesting.
 */
List *
build_index_pathkeys(Query *root, RelOptInfo *rel, IndexOptInfo *index)
{
	List	   *retval = NIL;
	int		   *indexkeys = index->indexkeys;
	Oid		   *ordering = index->ordering;

	if (!indexkeys || indexkeys[0] == 0 ||
		!ordering || ordering[0] == InvalidOid)
		return NIL;				/* unordered index? */

	if (index->indproc)
	{
		/* Functional index: build a representation of the function call */
		int			relid = lfirsti(rel->relids);
		Oid			reloid = getrelid(relid, root->rtable);
		Func	   *funcnode = makeNode(Func);
		List	   *funcargs = NIL;

		funcnode->funcid = index->indproc;
		funcnode->functype = get_func_rettype(index->indproc);
		funcnode->funcisindex = false;
		funcnode->funcsize = 0;
		funcnode->func_fcache = NULL;
		/* we assume here that the function returns a base type... */
		funcnode->func_tlist = setup_base_tlist(funcnode->functype);
		funcnode->func_planlist = NIL;

		while (*indexkeys != 0)
		{
			int			varattno = *indexkeys;
			Oid			vartypeid = get_atttype(reloid, varattno);
			int32		type_mod = get_atttypmod(reloid, varattno);

			funcargs = lappend(funcargs,
							   makeVar(relid, varattno, vartypeid,
									   type_mod, 0));
			indexkeys++;
		}

		/* Make a one-sublist pathkeys list for the function expression */
		retval = lcons(lcons(
			makePathKeyItem((Node *) make_funcclause(funcnode, funcargs),
							*ordering),
			NIL), NIL);
	}
	else
	{
		/* Normal non-functional index */
		List	   *rel_tlist = rel->targetlist;

		while (*indexkeys != 0 && *ordering != InvalidOid)
		{
			Var		*relvar = find_indexkey_var(*indexkeys, rel_tlist);

			/* If we can find no tlist entry for the n'th sort key,
			 * then we're done generating pathkeys; any subsequent sort keys
			 * no longer apply, since we can't represent the ordering properly
			 * even if there are tlist entries for them.
			 */
			if (!relvar)
				break;
			/* OK, make a one-element sublist for this sort key */
			retval = lappend(retval,
							 lcons(makePathKeyItem((Node *) relvar,
												   *ordering),
								   NIL));

			indexkeys++;
			ordering++;
		}
	}

	return retval;
}

/*
 * Find a var in a relation's targetlist that matches an indexkey attrnum.
 */
static Var *
find_indexkey_var(int indexkey, List *tlist)
{
	List	   *temp;

	foreach(temp, tlist)
	{
		Var	   *tle_var = get_expr(lfirst(temp));

		if (IsA(tle_var, Var) && tle_var->varattno == indexkey)
			return tle_var;
	}
	return NULL;
}

/*
 * build_join_pathkeys
 *	  Build the path keys for a join relation constructed by mergejoin or
 *	  nestloop join.  These keys should include all the path key vars of the
 *	  outer path (since the join will retain the ordering of the outer path)
 *	  plus any vars of the inner path that are mergejoined to the outer vars.
 *
 *	  Per the discussion at the top of this file, mergejoined inner vars
 *	  can be considered path keys of the result, just the same as the outer
 *	  vars they were joined with.
 *
 *	  We can also use inner path vars as pathkeys of a nestloop join, but we
 *	  must be careful that we only consider equijoin clauses and not general
 *	  join clauses.  For example, "t1.a < t2.b" might be a join clause of a
 *	  nestloop, but it doesn't result in b acquiring the ordering of a!
 *	  joinpath.c handles that problem by only passing this routine clauses
 *	  that are marked mergejoinable, even if a nestloop join is being built.
 *	  Therefore we only have 't1.a = t2.b' style clauses, and can expect that
 *	  the inner var will acquire the outer's ordering no matter which join
 *	  method is actually used.
 *
 *	  We drop pathkeys that are not vars of the join relation's tlist,
 *	  on the assumption that they are not interesting to higher levels.
 *	  (Is this correct??  To support expression pathkeys we might want to
 *	  check that all vars mentioned in the key are in the tlist, instead.)
 *
 * All vars in the result are taken from the join relation's tlist,
 * not from the given pathkeys or joinclauses.
 *
 * 'outer_pathkeys' is the list of the outer path's path keys
 * 'join_rel_tlist' is the target list of the join relation
 * 'joinclauses' is the list of mergejoinable clauses to consider (note this
 *		is a list of RestrictInfos, not just bare qual clauses); can be NIL
 *
 * Returns the list of new path keys.
 *
 */
List *
build_join_pathkeys(List *outer_pathkeys,
					List *join_rel_tlist,
					List *joinclauses)
{
	List	   *final_pathkeys = NIL;
	List	   *i;

	foreach(i, outer_pathkeys)
	{
		List	   *outer_pathkey = lfirst(i);
		List	   *new_pathkey;

		new_pathkey = build_join_pathkey(outer_pathkey, join_rel_tlist,
										 joinclauses);
		/* if we can find no sortable vars for the n'th sort key,
		 * then we're done generating pathkeys; any subsequent sort keys
		 * no longer apply, since we can't represent the ordering properly.
		 */
		if (new_pathkey == NIL)
			break;
		final_pathkeys = lappend(final_pathkeys, new_pathkey);
	}
	return final_pathkeys;
}

/*
 * build_join_pathkey
 *	  Generate an individual pathkey sublist, consisting of the outer vars
 *	  already mentioned in 'pathkey' plus any inner vars that are joined to
 *	  them (and thus can now also be considered path keys, per discussion
 *	  at the top of this file).
 *
 *	  Note that each returned pathkey uses the var node found in
 *	  'join_rel_tlist' rather than the input pathkey or joinclause var node.
 *	  (Is this important?)
 *
 * Returns a new pathkey (list of PathKeyItems).
 */
static List *
build_join_pathkey(List *pathkey,
				   List *join_rel_tlist,
				   List *joinclauses)
{
	List	   *new_pathkey = NIL;
	List	   *i,
			   *j;

	foreach(i, pathkey)
	{
		PathKeyItem *key = (PathKeyItem *) lfirst(i);
		Node	   *tlist_key;

		Assert(key && IsA(key, PathKeyItem));

		tlist_key = matching_tlist_expr(key->key, join_rel_tlist);
		if (tlist_key)
			new_pathkey = lcons(makePathKeyItem(tlist_key,
												key->sortop),
								new_pathkey);

		foreach(j, joinclauses)
		{
			RestrictInfo *restrictinfo = (RestrictInfo *) lfirst(j);
			Expr	   *joinclause = restrictinfo->clause;
			/* We assume the clause is a binary opclause... */
			Node	   *l = (Node *) get_leftop(joinclause);
			Node	   *r = (Node *) get_rightop(joinclause);
			Node	   *other_var = NULL;
			Oid			other_sortop = InvalidOid;

			if (equal(key->key, l))
			{
				other_var = r;
				other_sortop = restrictinfo->right_sortop;
			}
			else if (equal(key->key, r))
			{
				other_var = l;
				other_sortop = restrictinfo->left_sortop;
			}

			if (other_var && other_sortop)
			{
				tlist_key = matching_tlist_expr(other_var, join_rel_tlist);
				if (tlist_key)
					new_pathkey = lcons(makePathKeyItem(tlist_key,
														other_sortop),
										new_pathkey);
			}
		}
	}

	return new_pathkey;
}

/*
 * commute_pathkeys
 *		Attempt to commute the operators in a set of pathkeys, producing
 *		pathkeys that describe the reverse sort order (DESC instead of ASC).
 *		Returns TRUE if successful (all the operators have commutators).
 *
 * CAUTION: given pathkeys are modified in place, even if not successful!!
 * Usually, caller should have just built or copied the pathkeys list to
 * ensure there are no unwanted side-effects.
 */
bool
commute_pathkeys(List *pathkeys)
{
	List	   *i;

	foreach(i, pathkeys)
	{
		List	   *pathkey = lfirst(i);
		List	   *j;

		foreach(j, pathkey)
		{
			PathKeyItem	   *key = lfirst(j);

			key->sortop = get_commutator(key->sortop);
			if (key->sortop == InvalidOid)
				return false;
		}
	}
	return true;				/* successful */
}

/****************************************************************************
 *		PATHKEYS AND SORT CLAUSES
 ****************************************************************************/

/*
 * make_pathkeys_for_sortclauses
 *		Generate a pathkeys list that represents the sort order specified
 *		by a list of SortClauses (GroupClauses will work too!)
 *
 * 'sortclauses' is a list of SortClause or GroupClause nodes
 * 'tlist' is the targetlist to find the referenced tlist entries in
 */
List *
make_pathkeys_for_sortclauses(List *sortclauses, List *tlist)
{
	List	   *pathkeys = NIL;
	List	   *i;

	foreach(i, sortclauses)
	{
		SortClause	   *sortcl = (SortClause *) lfirst(i);
		Node		   *sortkey;
		PathKeyItem	   *pathkey;

		sortkey = get_sortgroupclause_expr(sortcl, tlist);
		pathkey = makePathKeyItem(sortkey, sortcl->sortop);
		/* pathkey becomes a one-element sublist */
		pathkeys = lappend(pathkeys, lcons(pathkey, NIL));
	}
	return pathkeys;
}

/****************************************************************************
 *		PATHKEYS AND MERGECLAUSES
 ****************************************************************************/

/*
 * find_mergeclauses_for_pathkeys
 *	  This routine attempts to find a set of mergeclauses that can be
 *	  used with a specified ordering for one of the input relations.
 *	  If successful, it returns a list of mergeclauses.
 *
 * 'pathkeys' is a pathkeys list showing the ordering of an input path.
 *			It doesn't matter whether it is for the inner or outer path.
 * 'restrictinfos' is a list of mergejoinable restriction clauses for the
 *			join relation being formed.
 *
 * The result is NIL if no merge can be done, else a maximal list of
 * usable mergeclauses (represented as a list of their restrictinfo nodes).
 *
 * XXX Ideally we ought to be considering context, ie what path orderings
 * are available on the other side of the join, rather than just making
 * an arbitrary choice among the mergeclause orders that will work for
 * this side of the join.
 */
List *
find_mergeclauses_for_pathkeys(List *pathkeys, List *restrictinfos)
{
	List	   *mergeclauses = NIL;
	List	   *i;

	foreach(i, pathkeys)
	{
		List		   *pathkey = lfirst(i);
		RestrictInfo   *matched_restrictinfo = NULL;
		List		   *j;

		/*
		 * We can match any of the keys in this pathkey sublist,
		 * since they're all equivalent.  And we can match against
		 * either left or right side of any mergejoin clause we haven't
		 * used yet.  For the moment we use a dumb "greedy" algorithm
		 * with no backtracking.  Is it worth being any smarter to
		 * make a longer list of usable mergeclauses?  Probably not.
		 */
		foreach(j, pathkey)
		{
			PathKeyItem	   *keyitem = lfirst(j);
			Node		   *key = keyitem->key;
			List		   *k;

			foreach(k, restrictinfos)
			{
				RestrictInfo   *restrictinfo = lfirst(k);

				Assert(restrictinfo->mergejoinoperator != InvalidOid);

				if ((equal(key, get_leftop(restrictinfo->clause)) ||
					 equal(key, get_rightop(restrictinfo->clause))) &&
					! member(restrictinfo, mergeclauses))
				{
					matched_restrictinfo = restrictinfo;
					break;
				}
			}
			if (matched_restrictinfo)
				break;
		}

		/*
		 * If we didn't find a mergeclause, we're done --- any additional
		 * sort-key positions in the pathkeys are useless.  (But we can
		 * still mergejoin if we found at least one mergeclause.)
		 */
		if (! matched_restrictinfo)
			break;
		/*
		 * If we did find a usable mergeclause for this sort-key position,
		 * add it to result list.
		 */
		mergeclauses = lappend(mergeclauses, matched_restrictinfo);
	}

	return mergeclauses;
}

/*
 * make_pathkeys_for_mergeclauses
 *	  Builds a pathkey list representing the explicit sort order that
 *	  must be applied to a path in order to make it usable for the
 *	  given mergeclauses.
 *
 * 'mergeclauses' is a list of RestrictInfos for mergejoin clauses
 *			that will be used in a merge join.
 * 'tlist' is a relation target list for either the inner or outer
 *			side of the proposed join rel.
 *
 * Returns a pathkeys list that can be applied to the indicated relation.
 *
 * Note that it is not this routine's job to decide whether sorting is
 * actually needed for a particular input path.  Assume a sort is necessary;
 * just make the keys, eh?
 */
List *
make_pathkeys_for_mergeclauses(List *mergeclauses, List *tlist)
{
	List	   *pathkeys = NIL;
	List	   *i;

	foreach(i, mergeclauses)
	{
		RestrictInfo *restrictinfo = (RestrictInfo *) lfirst(i);
		Node	   *key;
		Oid			sortop;

		Assert(restrictinfo->mergejoinoperator != InvalidOid);

		/*
		 * Find the key and sortop needed for this mergeclause.
		 *
		 * We can use either side of the mergeclause, since we haven't yet
		 * committed to which side will be inner.
		 */
		key = matching_tlist_expr((Node *) get_leftop(restrictinfo->clause),
								  tlist);
		sortop = restrictinfo->left_sortop;
		if (! key)
		{
			key = matching_tlist_expr((Node *) get_rightop(restrictinfo->clause),
									  tlist);
			sortop = restrictinfo->right_sortop;
		}
		if (! key)
			elog(ERROR, "make_pathkeys_for_mergeclauses: can't find key");
		/*
		 * Add a pathkey sublist for this sort item
		 */
		pathkeys = lappend(pathkeys,
						   lcons(makePathKeyItem(key, sortop),
								 NIL));
	}

	return pathkeys;
}
