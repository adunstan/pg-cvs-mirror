package org.postgresql.jdbc3;


import java.sql.*;

public class Jdbc3PreparedStatement extends org.postgresql.jdbc3.AbstractJdbc3Statement implements java.sql.PreparedStatement
{

	public Jdbc3PreparedStatement(Jdbc3Connection connection, String sql) throws SQLException
	{
		super(connection, sql);
	}

	public java.sql.ResultSet createResultSet (org.postgresql.Field[] fields, java.util.Vector tuples, String status, int updateCount, long insertOID, boolean binaryCursor) throws SQLException
	{
		return new Jdbc3ResultSet(this, fields, tuples, status, updateCount, insertOID, binaryCursor);
	}

}

