package org.postgresql.test.jdbc2;

import org.postgresql.test.JDBC2Tests;
import junit.framework.TestCase;
import java.io.*;
import java.sql.*;

import org.postgresql.largeobject.*;

/**
 * $Id: BlobTest.java,v 1.1 2001/02/14 17:45:17 peter Exp $
 *
 * Some simple tests based on problems reported by users. Hopefully these will
 * help prevent previous problems from re-occuring ;-)
 *
 */
public class BlobTest extends TestCase {

  public BlobTest(String name) {
    super(name);
  }

  /**
   * The table format used by this TestCase
   */
  private static final String BLOB_TABLE_FMT = "id name,lo oid";

  /**
   * Tests one method of uploading a blob to the database
   */
  public void testUploadBlob_LOOP() {
    try {
      Connection con = JDBC2Tests.openDB();

      JDBC2Tests.createTable(con,BLOB_TABLE_FMT);

      con.setAutoCommit(false);
      assert(!con.getAutoCommit());

      assert(uploadFile(con,"build.xml",LOOP)>0);

      // Now compare the blob & the file. Note this actually tests the
      // InputStream implementation!
      assert(compareBlobs(con));

      JDBC2Tests.closeDB(con);
    } catch(Exception ex) {
      assert(ex.getMessage(),false);
    }
  }

  /**
   * Tests one method of uploading a blob to the database
   */
  public void testUploadBlob_NATIVE() {
    try {
      Connection con = JDBC2Tests.openDB();

      JDBC2Tests.createTable(con,BLOB_TABLE_FMT);

      con.setAutoCommit(false);
      assert(!con.getAutoCommit());

      assert(uploadFile(con,"build.xml",NATIVE_STREAM)>0);

      // Now compare the blob & the file. Note this actually tests the
      // InputStream implementation!
      assert(compareBlobs(con));

      JDBC2Tests.closeDB(con);
    } catch(Exception ex) {
      assert(ex.getMessage(),false);
    }
  }

  private static final int LOOP          = 0; // LargeObject API using loop
  private static final int NATIVE_STREAM = 1; // LargeObject API using OutputStream
  private static final int JDBC_STREAM   = 2; // JDBC API using OutputStream

  /**
   * Helper - uploads a file into a blob using old style methods. We use this
   * because it always works, and we can use it as a base to test the new
   * methods.
   */
  private int uploadFile(Connection con,String file,int method) throws Exception {
    LargeObjectManager lom = ((org.postgresql.Connection)con).getLargeObjectAPI();

    FileInputStream fis = new FileInputStream(file);

    int oid = lom.create(LargeObjectManager.READWRITE);
    LargeObject blob = lom.open(oid);

    int s,t;
    byte buf[];
    OutputStream os;

    switch(method)
      {
        case LOOP:
          buf = new byte[2048];
          t=0;
          while((s=fis.read(buf,0,buf.length))>0) {
            t+=s;
            blob.write(buf,0,s);
          }
          break;

        case NATIVE_STREAM:
          os = blob.getOutputStream();
          s= fis.read();
          while(s>-1) {
            os.write(s);
            s=fis.read();
          }
          os.close();
          break;

        case JDBC_STREAM:
          File f = new File(file);
          PreparedStatement ps = con.prepareStatement(JDBC2Tests.insert("?"));
          ps.setBinaryStream(1,fis,(int) f.length());
          ps.execute();
          break;

        default:
          assert("Unknown method in uploadFile",false);
      }

    blob.close();
    fis.close();

    // Insert into the table
    Statement st = con.createStatement();
    st.executeUpdate(JDBC2Tests.insert("id,lo","'"+file+"',"+oid));
    con.commit();
    st.close();

    return oid;
  }

  /**
   * Helper - compares the blobs in a table with a local file. Note this alone
   * tests the InputStream methods!
   */
  private boolean compareBlobs(Connection con) throws Exception {
    boolean result=true;

    LargeObjectManager lom = ((org.postgresql.Connection)con).getLargeObjectAPI();

    Statement st = con.createStatement();
    ResultSet rs = st.executeQuery(JDBC2Tests.select("id,lo"));
    assert(rs!=null);

    while(rs.next()) {
      String file = rs.getString(1);
      int oid = rs.getInt(2);

      FileInputStream fis = new FileInputStream(file);
      LargeObject blob = lom.open(oid);
      InputStream bis = blob.getInputStream();

      int f=fis.read();
      int b=bis.read();
      int c=0;
      while(f>=0 && b>=0 & result) {
        result=(f==b);
        f=fis.read();
        b=bis.read();
        c++;
      }
      result=result && f==-1 && b==-1;

      if(!result)
        System.out.println("\nBlob compare failed at "+c+" of "+blob.size());

      blob.close();
      fis.close();
    }
    rs.close();
    st.close();

    return result;
  }
}
