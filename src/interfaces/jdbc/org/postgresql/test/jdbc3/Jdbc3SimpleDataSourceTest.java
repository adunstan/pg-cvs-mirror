package org.postgresql.test.jdbc3;

import java.sql.Connection;
import java.sql.SQLException;
import org.postgresql.test.jdbc2.optional.SimpleDataSourceTest;
import org.postgresql.test.TestUtil;
import org.postgresql.jdbc3.*;

/**
 * Tests JDBC3 non-pooling DataSource.
 *
 * @author Aaron Mulder (ammulder@chariotsolutions.com)
 * @version $Revision: 1.1.6.1 $
 */
public class Jdbc3SimpleDataSourceTest extends SimpleDataSourceTest {
    /**
     * Constructor required by JUnit
     */
    public Jdbc3SimpleDataSourceTest(String name)
    {
        super(name);
    }

    /**
     * Creates and configures a new SimpleDataSource.
     */
    protected void initializeDataSource()
    {
        if (bds == null)
        {
            bds = new Jdbc3SimpleDataSource();
            bds.setServerName(TestUtil.getServer());
            bds.setPortNumber(TestUtil.getPort());
            bds.setDatabaseName(TestUtil.getDatabase());
            bds.setUser(TestUtil.getUser());
            bds.setPassword(TestUtil.getPassword());
        }
    }
    /**
     * Makes sure this is a JDBC 3 implementation producing JDBC3
     * connections.
     */
    public void testConfirmJdbc3Impl()
    {
        try {
            Connection con = getDataSourceConnection();
            assertTrue("Wrong SimpleDataSource impl used by test: "+bds.getClass().getName(), bds instanceof Jdbc3SimpleDataSource);
            assertTrue("Wrong Connnection class generated by JDBC3 DataSource: "+con.getClass().getName(), con instanceof Jdbc3Connection);
        } catch (SQLException e) {
            fail(e.getMessage());
        }
    }
}
