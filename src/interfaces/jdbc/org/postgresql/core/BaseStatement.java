/*-------------------------------------------------------------------------
 *
 * BaseStatement.java
 *	  The internal interface definition for a jdbc statement
 *
 * Copyright (c) 2003, PostgreSQL Global Development Group
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/interfaces/jdbc/org/postgresql/core/Attic/BaseStatement.java,v 1.1 2003/03/07 18:39:41 barry Exp $
 *
 *-------------------------------------------------------------------------
 */
package org.postgresql.core;


import java.sql.*;
import java.util.Vector;

public interface BaseStatement extends org.postgresql.PGStatement
{
	public BaseResultSet createResultSet(Field[] fields, Vector tuples, String status, int updateCount, long insertOID, boolean binaryCursor) throws SQLException;

	public BaseConnection getPGConnection();

	/*
	 * The maxRows limit is set to limit the number of rows that
	 * any ResultSet can contain.  If the limit is exceeded, the
	 * excess rows are silently dropped.
	 */
	public int getFetchSize() throws SQLException;
	public int getMaxRows() throws SQLException;
	public int getResultSetConcurrency() throws SQLException;
	public String getStatementName();

}
