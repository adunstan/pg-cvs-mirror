package org.postgresql;


import java.sql.*;

/* $Header: /home/cvsmirror/pg/pgsql/src/interfaces/jdbc/org/postgresql/Attic/PGStatement.java,v 1.4 2002/09/02 03:07:36 barry Exp $
 * This interface defines PostgreSQL extentions to the java.sql.Statement interface.
 * Any java.sql.Statement object returned by the driver will also implement this 
 * interface
 */
public interface PGStatement
{

	/*
	 * Returns the Last inserted/updated oid. 
	 * @return OID of last insert
         * @since 7.3
	 */
        public long getLastOID() throws SQLException;

        public void setUseServerPrepare(boolean flag);

        public boolean isUseServerPrepare();

}
