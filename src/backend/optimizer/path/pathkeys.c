/*-------------------------------------------------------------------------
 *
 * joinutils.c
 *	  Utilities for matching and building join and path keys
 *
 * Copyright (c) 1994, Regents of the University of California
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/optimizer/path/pathkeys.c,v 1.6 1999/02/21 01:55:02 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include "nodes/pg_list.h"
#include "nodes/relation.h"
#include "nodes/plannodes.h"

#include "optimizer/internal.h"
#include "optimizer/paths.h"
#include "optimizer/var.h"
#include "optimizer/keys.h"
#include "optimizer/tlist.h"
#include "optimizer/joininfo.h"
#include "optimizer/ordering.h"

static int match_pathkey_joinkeys(List *pathkey, List *joinkeys,
						int outer_or_inner);
static List *new_join_pathkey(List *subkeys, List *considered_subkeys,
					 	List *join_rel_tlist, List *joinclauses);
static List *new_matching_subkeys(Var *subkey, List *considered_subkeys,
					 	List *join_rel_tlist, List *joinclauses);


/*
 *	Explanation of Path.pathkeys
 *
 *	Path.pathkeys is a List of List of Var nodes that represent the sort
 *	order of the result generated by the Path.
 *
 *	In single/base relation RelOptInfo's, the Path's represent various ways
 *	of generate the relation.  Sequential scan Paths have a NIL pathkeys.
 *	Index scans have Path.pathkeys that represent the chosen index.  A
 *	single-key index pathkeys would be { {tab1_indexkey1} }.  The pathkeys
 *	entry for a multi-key index would be { {tab1_indexkey1}, {tab1_indexkey2} }.
 *
 *	Multi-relation RelOptInfo Path's are more complicated.  Mergejoins are
 *	only performed with equajoins("=").  Because of this, the multi-relation
 *	path actually has more than one primary Var key.  For example, a
 *	mergejoin Path of "tab1.col1 = tab2.col1" would generate a pathkeys of
 *	{ {tab1.col1, tab2.col1} }.  This allows future joins to use either Var
 *	as a pre-sorted key to prevent Mergejoins from having to re-sort the Path.
 *	They are equal, so they are both primary sort keys.  This is why pathkeys
 *	is a List of Lists.
 *
 *	For multi-key sorts, if the outer is sorted by a multi-key index, the
 *	multi-key index remains after the join.  If the inner has a multi-key
 *	sort, only the primary key of the inner is added to the result.
 *	Mergejoins only join on the primary key.  Currently, non-primary keys
 *	in the pathkeys List are of limited value.
 *
 *	HashJoin has similar functionality.  NestJoin does not perform sorting,
 *	and allows non-equajoins, so it does not allow useful pathkeys.
 *
 *	-- bjm
 *	
 */
 
/****************************************************************************
 *		KEY COMPARISONS
 ****************************************************************************/

/*
 * order_joinkeys_by_pathkeys
 *	  Attempts to match the keys of a path against the keys of join clauses.
 *	  This is done by looking for a matching join key in 'joinkeys' for
 *	  every path key in the list 'path.keys'. If there is a matching join key
 *	  (not necessarily unique) for every path key, then the list of
 *	  corresponding join keys and join clauses are returned in the order in
 *	  which the keys matched the path keys.
 *
 * 'pathkeys' is a list of path keys:
 *		( ( (var) (var) ... ) ( (var) ... ) )
 * 'joinkeys' is a list of join keys:
 *		( (outer inner) (outer inner) ... )
 * 'joinclauses' is a list of clauses corresponding to the join keys in
 *		'joinkeys'
 * 'outer_or_inner' is a flag that selects the desired subkey of a join key
 *		in 'joinkeys'
 *
 * Returns the join keys and corresponding join clauses in a list if all
 * of the path keys were matched:
 *		(
 *		 ( (outerkey0 innerkey0) ... (outerkeyN or innerkeyN) )
 *		 ( clause0 ... clauseN )
 *		)
 * and nil otherwise.
 *
 * Returns a list of matched join keys and a list of matched join clauses
 * in pointers if valid order can be found.
 */
bool
order_joinkeys_by_pathkeys(List *pathkeys,
							List *joinkeys,
							List *joinclauses,
							int outer_or_inner,
							List **matchedJoinKeysPtr,
							List **matchedJoinClausesPtr)
{
	List	   *matched_joinkeys = NIL;
	List	   *matched_joinclauses = NIL;
	List	   *pathkey = NIL;
	List	   *i = NIL;
	int			matched_joinkey_index = -1;
	int			matched_keys = 0;
	/*
	 *	Reorder the joinkeys by picking out one that matches each pathkey,
	 *	and create a new joinkey/joinclause list in pathkey order
	 */
	foreach(i, pathkeys)
	{
		pathkey = lfirst(i);
		matched_joinkey_index = match_pathkey_joinkeys(pathkey, joinkeys,
													   outer_or_inner);

		if (matched_joinkey_index != -1)
		{
			matched_keys++;
			if (matchedJoinKeysPtr)
			{
				JoinKey	   *joinkey = nth(matched_joinkey_index, joinkeys);
				matched_joinkeys = lappend(matched_joinkeys, joinkey);
			}
			
			if (matchedJoinClausesPtr && joinclauses)
			{
				Expr	   *joinclause = nth(matched_joinkey_index,
											 joinclauses);
				matched_joinclauses = lappend(matched_joinclauses, joinclause);
			}
		}
		else
			/*	A pathkey could not be matched. */
			break;
	}

	/*
	 *	Did we fail to match all the joinkeys?
	 *	Extra pathkeys are no problem.
	 */
	if (matched_keys != length(joinkeys))
	{
			if (matchedJoinKeysPtr)
				*matchedJoinKeysPtr = NIL;
			if (matchedJoinClausesPtr)
				*matchedJoinClausesPtr = NIL;
			return false;
	}

	if (matchedJoinKeysPtr)
		*matchedJoinKeysPtr = matched_joinkeys;
	if (matchedJoinClausesPtr)
		*matchedJoinClausesPtr = matched_joinclauses;
	return true;
}


/*
 * match_pathkey_joinkeys
 *	  Returns the 0-based index into 'joinkeys' of the first joinkey whose
 *	  outer or inner subkey matches any subkey of 'pathkey'.
 *
 *	All these keys are equivalent, so any of them can match.  See above.
 */
static int
match_pathkey_joinkeys(List *pathkey,
					   List *joinkeys,
					   int outer_or_inner)
{
	Var		   *path_subkey;
	int			pos;
	List	   *i, *x;
	JoinKey    *jk;

	foreach(i, pathkey)
	{
		path_subkey = (Var *) lfirst(i);
		pos = 0;
		foreach(x, joinkeys)
		{
			jk = (JoinKey *) lfirst(x);
			if (var_equal(path_subkey, extract_join_key(jk, outer_or_inner)))
				return pos;
			pos++;
		}
	}
	return -1;					/* no index found	*/
}


/*
 * get_cheapest_path_for_joinkeys
 *	  Attempts to find a path in 'paths' whose keys match a set of join
 *	  keys 'joinkeys'.	To match,
 *	  1. the path node ordering must equal 'ordering'.
 *	  2. each subkey of a given path must match(i.e., be(var_equal) to) the
 *		 appropriate subkey of the corresponding join key in 'joinkeys',
 *		 i.e., the Nth path key must match its subkeys against the subkey of
 *		 the Nth join key in 'joinkeys'.
 *
 * 'joinkeys' is the list of key pairs to which the path keys must be
 *		matched
 * 'ordering' is the ordering of the(outer) path to which 'joinkeys'
 *		must correspond
 * 'paths' is a list of(inner) paths which are to be matched against
 *		each join key in 'joinkeys'
 * 'outer_or_inner' is a flag that selects the desired subkey of a join key
 *		in 'joinkeys'
 *
 *	Find the cheapest path that matches the join keys
 */
Path *
get_cheapest_path_for_joinkeys(List *joinkeys,
								 PathOrder *ordering,
								 List *paths,
								 int outer_or_inner)
{
	Path	   *matched_path = NULL;
	List	   *i = NIL;

	foreach(i, paths)
	{
		Path	   *path = (Path *) lfirst(i);
		int			better_sort, better_key;
		
		if (order_joinkeys_by_pathkeys(path->pathkeys, joinkeys, NIL,
									   outer_or_inner, NULL, NULL) &&
			pathorder_match(ordering, path->pathorder, &better_sort) &&
			better_sort == 0)
		{
			if (matched_path)
				if (path->path_cost < matched_path->path_cost)
					matched_path = path;
			else
				matched_path = path;
		}
	}
	return matched_path;
}


/*
 * extract_path_keys
 *	  Builds a subkey list for a path by pulling one of the subkeys from
 *	  a list of join keys 'joinkeys' and then finding the var node in the
 *	  target list 'tlist' that corresponds to that subkey.
 *
 * 'joinkeys' is a list of join key pairs
 * 'tlist' is a relation target list
 * 'outer_or_inner' is a flag that selects the desired subkey of a join key
 *	in 'joinkeys'
 *
 * Returns a list of pathkeys: ((tlvar1)(tlvar2)...(tlvarN)).
 * It is a list of lists because of multi-key indexes.
 */
List *
extract_path_keys(List *joinkeys,
				  List *tlist,
				  int outer_or_inner)
{
	List	   *pathkeys = NIL;
	List	   *jk;

	foreach(jk, joinkeys)
	{
		JoinKey    *jkey = (JoinKey *) lfirst(jk);
		Var		   *var,
				   *key;
		List	   *p;

		/*
		 * find the right Var in the target list for this key
		 */
		var = (Var *) extract_join_key(jkey, outer_or_inner);
		key = (Var *) matching_tlist_var(var, tlist);

		/*
		 * Include it in the pathkeys list if we haven't already done so
		 */
		foreach(p, pathkeys)
		{
			Var		   *pkey = lfirst((List *) lfirst(p));		/* XXX fix me */

			if (key == pkey)
				break;
		}
		if (p != NIL)
			continue;			/* key already in pathkeys */

		pathkeys = lappend(pathkeys, lcons(key, NIL));
	}
	return pathkeys;
}


/****************************************************************************
 *		NEW PATHKEY FORMATION
 ****************************************************************************/

/*
 * new_join_pathkeys
 *	  Find the path keys for a join relation by finding all vars in the list
 *	  of join clauses 'joinclauses' such that:
 *		(1) the var corresponding to the outer join relation is a
 *			key on the outer path
 *		(2) the var appears in the target list of the join relation
 *	  In other words, add to each outer path key the inner path keys that
 *	  are required for qualification.
 *
 * 'outer_pathkeys' is the list of the outer path's path keys
 * 'join_rel_tlist' is the target list of the join relation
 * 'joinclauses' is the list of restricting join clauses
 *
 * Returns the list of new path keys.
 *
 */
List *
new_join_pathkeys(List *outer_pathkeys,
				  List *join_rel_tlist,
				  List *joinclauses)
{
	List	   *outer_pathkey = NIL;
	List	   *t_list = NIL;
	List	   *x;
	List	   *i = NIL;

	foreach(i, outer_pathkeys)
	{
		outer_pathkey = lfirst(i);
		x = new_join_pathkey(outer_pathkey, NIL, join_rel_tlist, joinclauses);
		if (x != NIL)
			t_list = lappend(t_list, x);
	}
	return t_list;
}

/*
 * new_join_pathkey
 *	  Finds new vars that become subkeys due to qualification clauses that
 *	  contain any previously considered subkeys.  These new subkeys plus the
 *	  subkeys from 'subkeys' form a new pathkey for the join relation.
 *
 *	  Note that each returned subkey is the var node found in
 *	  'join_rel_tlist' rather than the joinclause var node.
 *
 * 'subkeys' is a list of subkeys for which matching subkeys are to be
 *		found
 * 'considered_subkeys' is the current list of all subkeys corresponding
 *		to a given pathkey
 *
 * Returns a new pathkey(list of subkeys).
 *
 */
static List *
new_join_pathkey(List *subkeys,
				 List *considered_subkeys,
				 List *join_rel_tlist,
				 List *joinclauses)
{
	List	   *t_list = NIL;
	Var		   *subkey;
	List	   *i = NIL;
	List	   *matched_subkeys = NIL;
	Expr	   *tlist_key = (Expr *) NULL;
	List	   *newly_considered_subkeys = NIL;

	foreach(i, subkeys)
	{
		subkey = (Var *) lfirst(i);
		if (subkey == NULL)
			break;				/* XXX something is wrong */
		matched_subkeys = new_matching_subkeys(subkey, considered_subkeys,
								 join_rel_tlist, joinclauses);
		tlist_key = matching_tlist_var(subkey, join_rel_tlist);
		newly_considered_subkeys = NIL;

		if (tlist_key)
		{
			if (!member(tlist_key, matched_subkeys))
				newly_considered_subkeys = lcons(tlist_key, matched_subkeys);
		}
		else
			newly_considered_subkeys = matched_subkeys;

		considered_subkeys =  append(considered_subkeys, newly_considered_subkeys);

		t_list = nconc(t_list, newly_considered_subkeys);
	}
	return t_list;
}

/*
 * new_matching_subkeys
 *	  Returns a list of new subkeys:
 *	  (1) which are not listed in 'considered_subkeys'
 *	  (2) for which the "other" variable in some clause in 'joinclauses' is
 *		  'subkey'
 *	  (3) which are mentioned in 'join_rel_tlist'
 *
 *	  Note that each returned subkey is the var node found in
 *	  'join_rel_tlist' rather than the joinclause var node.
 *
 * 'subkey' is the var node for which we are trying to find matching
 *		clauses
 *
 * Returns a list of new subkeys.
 *
 */
static List *
new_matching_subkeys(Var *subkey,
					 List *considered_subkeys,
					 List *join_rel_tlist,
					 List *joinclauses)
{
	List	   *t_list = NIL;
	Expr	   *joinclause;
	List	   *i;
	Expr	   *tlist_other_var;

	foreach(i, joinclauses)
	{
		joinclause = lfirst(i);
		tlist_other_var = matching_tlist_var(
									other_join_clause_var(subkey, joinclause),
									join_rel_tlist);

		if (tlist_other_var &&
			!(member(tlist_other_var, considered_subkeys)))
		{

			/* XXX was "push" function	*/
			considered_subkeys = lappend(considered_subkeys,
										 tlist_other_var);

			/*
			 * considered_subkeys = nreverse(considered_subkeys); XXX -- I
			 * am not sure of this.
			 */

			t_list = lappend(t_list, tlist_other_var);
		}
	}
	return t_list;
}
