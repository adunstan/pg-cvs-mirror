package org.postgresql.jdbc2;

import org.postgresql.jdbc1.AbstractJdbc1ResultSet;
import java.sql.*;
import java.util.*;

import org.postgresql.Driver;
import org.postgresql.Field;
import org.postgresql.util.PSQLException;

public abstract class AbstractJdbc2DatabaseMetaData extends org.postgresql.jdbc1.AbstractJdbc1DatabaseMetaData
{

	public AbstractJdbc2DatabaseMetaData(AbstractJdbc2Connection conn)
	{
	    super(conn);
	}



	// ** JDBC 2 Extensions **

	/*
	 * Does the database support the given result set type?
	 *
	 * @param type - defined in java.sql.ResultSet
	 * @return true if so; false otherwise
	 * @exception SQLException - if a database access error occurs
	 */
	public boolean supportsResultSetType(int type) throws SQLException
	{
		// The only type we don't support
		return type != java.sql.ResultSet.TYPE_SCROLL_SENSITIVE;
	}


	/*
	 * Does the database support the concurrency type in combination
	 * with the given result set type?
	 *
	 * @param type - defined in java.sql.ResultSet
	 * @param concurrency - type defined in java.sql.ResultSet
	 * @return true if so; false otherwise
	 * @exception SQLException - if a database access error occurs
	*/
	public boolean supportsResultSetConcurrency(int type, int concurrency) throws SQLException
	{
		// These combinations are not supported!
		if (type == java.sql.ResultSet.TYPE_SCROLL_SENSITIVE)
			return false;

		// We don't yet support Updateable ResultSets
		if (concurrency == java.sql.ResultSet.CONCUR_UPDATABLE)
			return false;

		// Everything else we do
		return true;
	}


	/* lots of unsupported stuff... */
	public boolean ownUpdatesAreVisible(int type) throws SQLException
	{
		return false;
	}

	public boolean ownDeletesAreVisible(int type) throws SQLException
	{
		return false;
	}

	public boolean ownInsertsAreVisible(int type) throws SQLException
	{
		return false;
	}

	public boolean othersUpdatesAreVisible(int type) throws SQLException
	{
		return false;
	}

	public boolean othersDeletesAreVisible(int i) throws SQLException
	{
		return false;
	}

	public boolean othersInsertsAreVisible(int type) throws SQLException
	{
		return false;
	}

	public boolean updatesAreDetected(int type) throws SQLException
	{
		return false;
	}

	public boolean deletesAreDetected(int i) throws SQLException
	{
		return false;
	}

	public boolean insertsAreDetected(int type) throws SQLException
	{
		return false;
	}

	/*
	 * Indicates whether the driver supports batch updates.
	 */
	public boolean supportsBatchUpdates() throws SQLException
	{
		return true;
	}

	/*
	 * Return user defined types in a schema
	 */
	public java.sql.ResultSet getUDTs(String catalog,
                                          String schemaPattern,
                                          String typeNamePattern,
                                          int[] types
                                         ) throws SQLException
	{
		throw org.postgresql.Driver.notImplemented();
	}


	/*
	 * Retrieves the connection that produced this metadata object.
	 *
	 * @return the connection that produced this metadata object
	 */
	public java.sql.Connection getConnection() throws SQLException
	{
		return (java.sql.Connection)connection;
	}

	/* I don't find these in the spec!?! */

	public boolean rowChangesAreDetected(int type) throws SQLException
	{
		return false;
	}

	public boolean rowChangesAreVisible(int type) throws SQLException
	{
		return false;
	}
}
