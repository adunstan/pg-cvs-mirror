package org.postgresql;

import java.sql.*;
import org.postgresql.util.PSQLException;

/**
 * This class defines methods implemented by the two subclasses
 * org.postgresql.jdbc1.Statement and org.postgresql.jdbc2.Statement that are
 * unique to PostgreSQL's JDBC driver.
 *
 * <p>They are defined so that client code can cast to org.postgresql.Statement
 * without having to predetermine the jdbc driver type.
 *
 * <p>ie: Before this class existed, you had to use:
 *
 * <p>((org.postgresql.jdbc2.Statement)stat).getInsertedOID();
 *
 * <p>now you use:
 *
 * <p>((org.postgresql.Statement)stat).getInsertedOID();
 *
 * <p>As you can see, this is independent of JDBC1.2, JDBC2.0 or the upcoming
 * JDBC3.
 */

public abstract class Statement {

    /** The warnings chain. */
    protected SQLWarning warnings = null;

    /** The current results */
    protected java.sql.ResultSet result = null;

    /** Maximum number of rows to return, 0 = unlimited */
    protected int maxrows = 0;  

    /** Timeout (in seconds) for a query (not used) */
    protected int timeout = 0;

    protected boolean escapeProcessing = true;


    public Statement() {
    }

    /**
     * Returns the status message from the current Result.<p>
     * This is used internally by the driver.
     *
     * @return status message from backend
     */
    public String getResultStatusString() {
	if (result == null)
	    return null;
	return ((org.postgresql.ResultSet) result).getStatusString();
    }

    /**
     * The maxRows limit is set to limit the number of rows that
     * any ResultSet can contain.  If the limit is exceeded, the
     * excess rows are silently dropped.
     *
     * @return the current maximum row limit; zero means unlimited
     * @exception SQLException if a database access error occurs
     */
    public int getMaxRows() throws SQLException {
	return maxrows;
    }

    /**
     * Set the maximum number of rows
     *
     * @param max the new max rows limit; zero means unlimited
     * @exception SQLException if a database access error occurs
     * @see getMaxRows
     */
    public void setMaxRows(int max) throws SQLException {
	maxrows = max;
    }

    /**
     * If escape scanning is on (the default), the driver will do escape
     * substitution before sending the SQL to the database.
     *
     * @param enable true to enable; false to disable
     * @exception SQLException if a database access error occurs
     */
    public void setEscapeProcessing(boolean enable) throws SQLException {
	escapeProcessing = enable;
    }

    /**
     * The queryTimeout limit is the number of seconds the driver
     * will wait for a Statement to execute.  If the limit is
     * exceeded, a SQLException is thrown.
     *
     * @return the current query timeout limit in seconds; 0 = unlimited
     * @exception SQLException if a database access error occurs
     */
    public int getQueryTimeout() throws SQLException {
	return timeout;
    }

    /**
     * Sets the queryTimeout limit
     *
     * @param seconds - the new query timeout limit in seconds
     * @exception SQLException if a database access error occurs
     */
    public void setQueryTimeout(int seconds) throws SQLException {
	timeout = seconds;
    }

    /**
     * The first warning reported by calls on this Statement is
     * returned.  A Statement's execute methods clear its SQLWarning
     * chain.  Subsequent Statement warnings will be chained to this
     * SQLWarning.
     *
     * <p>The Warning chain is automatically cleared each time a statement
     * is (re)executed.
     *
     * <p><B>Note:</B>  If you are processing a ResultSet then any warnings
     * associated with ResultSet reads will be chained on the ResultSet
     * object.
     *
     * @return the first SQLWarning on null
     * @exception SQLException if a database access error occurs
     */
    public SQLWarning getWarnings() throws SQLException {
	return warnings;
    }

    /**
     * The maxFieldSize limit (in bytes) is the maximum amount of
     * data returned for any column value; it only applies to
     * BINARY, VARBINARY, LONGVARBINARY, CHAR, VARCHAR and LONGVARCHAR
     * columns.  If the limit is exceeded, the excess data is silently
     * discarded.
     *
     * @return the current max column size limit; zero means unlimited
     * @exception SQLException if a database access error occurs
     */
    public int getMaxFieldSize() throws SQLException {
	return 8192;		// We cannot change this
    }

    /**
     * Sets the maxFieldSize - NOT! - We throw an SQLException just
     * to inform them to stop doing this.
     *
     * @param max the new max column size limit; zero means unlimited
     * @exception SQLException if a database access error occurs
     */
    public void setMaxFieldSize(int max) throws SQLException {
	throw new PSQLException("postgresql.stat.maxfieldsize");
    }

    /**
     * After this call, getWarnings returns null until a new warning
     * is reported for this Statement.
     *
     * @exception SQLException if a database access error occurs
     */
    public void clearWarnings() throws SQLException {
	warnings = null;
    }

    /**
     * Cancel can be used by one thread to cancel a statement that
     * is being executed by another thread.
     * <p>
     * Not implemented, this method is a no-op.
     *
     * @exception SQLException only because thats the spec.
     */
    public void cancel() throws SQLException {
	// FIXME: Cancel feature has been available since 6.4. Implement it here!
    }

    /**
     * New in 7.1: Returns the Last inserted oid. This should be used, rather
     * than the old method using getResultSet, which for executeUpdate returns
     * null.
     * @return OID of last insert
     */
    public int getInsertedOID() throws SQLException {
	if (result == null)
	    return 0;
	return ((org.postgresql.ResultSet) result).getInsertedOID();
    }

    /**
     * getResultSet returns the current result as a ResultSet.  It
     * should only be called once per result.
     *
     * @return the current result set; null if there are no more
     * @exception SQLException if a database access error occurs (why?)
     */
    public java.sql.ResultSet getResultSet() throws SQLException {
	if (result != null && ((org.postgresql.ResultSet) result).reallyResultSet())
            return result;
	return null;
    }

    /**
     * In many cases, it is desirable to immediately release a
     * Statement's database and JDBC resources instead of waiting
     * for this to happen when it is automatically closed.  The
     * close method provides this immediate release.
     *
     * <p><B>Note:</B> A Statement is automatically closed when it is
     * garbage collected.  When a Statement is closed, its current
     * ResultSet, if one exists, is also closed.
     *
     * @exception SQLException if a database access error occurs (why?)
     */
    public void close() throws SQLException {
	// Force the ResultSet to close
	java.sql.ResultSet rs = getResultSet();
	if(rs!=null)
            rs.close();

	// Disasociate it from us (For Garbage Collection)
	result = null;
    }

    /**
     * This is an attempt to implement SQL Escape clauses
     */
    protected static String escapeSQL(String sql) {
      // If we find a "{d", assume we have a date escape.
      //
      // Since the date escape syntax is very close to the
      // native Postgres date format, we just remove the escape
      // delimiters.
      //
      // This implementation could use some optimization, but it has
      // worked in practice for two years of solid use.
      int index = sql.indexOf("{d");
      while (index != -1) {
        StringBuffer buf = new StringBuffer(sql);
        buf.setCharAt(index, ' ');
        buf.setCharAt(index + 1, ' ');
        buf.setCharAt(sql.indexOf('}', index), ' ');
        sql = new String(buf);
        index = sql.indexOf("{d");
      }
      return sql;
    }
}
