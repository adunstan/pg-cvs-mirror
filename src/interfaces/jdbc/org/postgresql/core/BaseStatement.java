/*-------------------------------------------------------------------------
 *
 * BaseStatement.java
 *	  The internal interface definition for a jdbc statement
 *
 * Copyright (c) 2003, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/interfaces/jdbc/org/postgresql/core/Attic/BaseStatement.java,v 1.4.2.1 2003/09/07 04:37:10 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */
package org.postgresql.core;

import org.postgresql.PGRefCursorResultSet;
import java.sql.*;
import java.util.Vector;

public interface BaseStatement extends org.postgresql.PGStatement
{
        public BaseResultSet createResultSet(Field[] fields, Vector tuples, String status, int updateCount, long insertOID, boolean binaryCursor) throws SQLException;
        public PGRefCursorResultSet createRefCursorResultSet(String cursorName) throws SQLException;

	public BaseConnection getPGConnection();

	/*
	 * The maxRows limit is set to limit the number of rows that
	 * any ResultSet can contain.  If the limit is exceeded, the
	 * excess rows are silently dropped.
	 */
	public void addWarning(String p_warning) throws SQLException;
	public void close() throws SQLException;
	public int getFetchSize() throws SQLException;
 	public int getMaxFieldSize() throws SQLException;
	public int getMaxRows() throws SQLException;
	public int getResultSetConcurrency() throws SQLException;
	public String getStatementName();
	public SQLWarning getWarnings() throws SQLException;
 	public void setMaxFieldSize(int max) throws SQLException;

}
