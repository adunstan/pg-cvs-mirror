package postgresql.jdbc1;

// IMPORTANT NOTE: This file implements the JDBC 1 version of the driver.
// If you make any modifications to this file, you must make sure that the
// changes are also made (if relevent) to the related JDBC 2 class in the
// postgresql.jdbc2 package.

import java.lang.*;
import java.io.*;
import java.math.*;
import java.text.*;
import java.util.*;
import java.sql.*;
import postgresql.Field;
import postgresql.largeobject.*;
import postgresql.util.*;

/**
 * A ResultSet provides access to a table of data generated by executing a
 * Statement.  The table rows are retrieved in sequence.  Within a row its
 * column values can be accessed in any order.
 *
 * <P>A ResultSet maintains a cursor pointing to its current row of data.  
 * Initially the cursor is positioned before the first row.  The 'next'
 * method moves the cursor to the next row.
 *
 * <P>The getXXX methods retrieve column values for the current row.  You can
 * retrieve values either using the index number of the column, or by using
 * the name of the column.  In general using the column index will be more
 * efficient.  Columns are numbered from 1.
 *
 * <P>For maximum portability, ResultSet columns within each row should be read
 * in left-to-right order and each column should be read only once.
 *
 *<P> For the getXXX methods, the JDBC driver attempts to convert the
 * underlying data to the specified Java type and returns a suitable Java
 * value.  See the JDBC specification for allowable mappings from SQL types
 * to Java types with the ResultSet getXXX methods.
 *
 * <P>Column names used as input to getXXX methods are case insenstive.  When
 * performing a getXXX using a column name, if several columns have the same
 * name, then the value of the first matching column will be returned.  The
 * column name option is designed to be used when column names are used in the
 * SQL Query.  For columns that are NOT explicitly named in the query, it is
 * best to use column numbers.  If column names were used there is no way for
 * the programmer to guarentee that they actually refer to the intended
 * columns.
 *
 * <P>A ResultSet is automatically closed by the Statement that generated it 
 * when that Statement is closed, re-executed, or is used to retrieve the 
 * next result from a sequence of multiple results.
 *
 * <P>The number, types and properties of a ResultSet's columns are provided by
 * the ResultSetMetaData object returned by the getMetaData method.
 *
 * @see ResultSetMetaData
 * @see java.sql.ResultSet
 */
public class ResultSet extends postgresql.ResultSet implements java.sql.ResultSet 
{
  /**
   * Create a new ResultSet - Note that we create ResultSets to
   * represent the results of everything.
   *
   * @param fields an array of Field objects (basically, the
   *	ResultSet MetaData)
   * @param tuples Vector of the actual data
   * @param status the status string returned from the back end
   * @param updateCount the number of rows affected by the operation
   * @param cursor the positioned update/delete cursor name
   */
  public ResultSet(Connection conn, Field[] fields, Vector tuples, String status, int updateCount)
  {
      super(conn,fields,tuples,status,updateCount);
  }
  
  /**
   * A ResultSet is initially positioned before its first row,
   * the first call to next makes the first row the current row;
   * the second call makes the second row the current row, etc.
   *
   * <p>If an input stream from the previous row is open, it is
   * implicitly closed.  The ResultSet's warning chain is cleared
   * when a new row is read
   *
   * @return true if the new current is valid; false if there are no
   *	more rows
   * @exception SQLException if a database access error occurs
   */
  public boolean next() throws SQLException
  {
    if (++current_row >= rows.size())
      return false;
    this_row = (byte [][])rows.elementAt(current_row);
    return true;
  }
  
  /**
   * In some cases, it is desirable to immediately release a ResultSet
   * database and JDBC resources instead of waiting for this to happen
   * when it is automatically closed.  The close method provides this
   * immediate release.
   *
   * <p><B>Note:</B> A ResultSet is automatically closed by the Statement
   * the Statement that generated it when that Statement is closed,
   * re-executed, or is used to retrieve the next result from a sequence
   * of multiple results.  A ResultSet is also automatically closed 
   * when it is garbage collected.
   *
   * @exception SQLException if a database access error occurs
   */
  public void close() throws SQLException
  {
    // No-op
  }
  
  /**
   * A column may have the value of SQL NULL; wasNull() reports whether
   * the last column read had this special value.  Note that you must
   * first call getXXX on a column to try to read its value and then
   * call wasNull() to find if the value was SQL NULL
   *
   * @return true if the last column read was SQL NULL
   * @exception SQLException if a database access error occurred
   */
  public boolean wasNull() throws SQLException
  {
    return wasNullFlag;
  }
  
  /**
   * Get the value of a column in the current row as a Java String
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return the column value, null for SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public String getString(int columnIndex) throws SQLException
  {
    //byte[] bytes = getBytes(columnIndex);
    //
    //if (bytes == null)
    //return null;
    //return new String(bytes);
    if (columnIndex < 1 || columnIndex > fields.length)
      throw new PSQLException("postgresql.res.colrange");
    wasNullFlag = (this_row[columnIndex - 1] == null);
    if(wasNullFlag)
      return null;
    return new String(this_row[columnIndex - 1]);
  }
  
  /**
   * Get the value of a column in the current row as a Java boolean
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return the column value, false for SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public boolean getBoolean(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	int c = s.charAt(0);
	return ((c == 't') || (c == 'T'));
      }
    return false;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java byte.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public byte getByte(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Byte.parseByte(s);
	  } catch (NumberFormatException e) {
	    throw new PSQLException("postgresql.res.badbyte",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java short.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public short getShort(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Short.parseShort(s);
	  } catch (NumberFormatException e) {
	    throw new PSQLException("postgresql.res.badshort",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java int.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public int getInt(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Integer.parseInt(s);
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.badint",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java long.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public long getLong(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Long.parseLong(s);
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.res.badlong",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java float.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public float getFloat(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Float.valueOf(s).floatValue();
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.res.badfloat",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java double.
   *
   * @param columnIndex the first column is 1, the second is 2,...
   * @return the column value; 0 if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public double getDouble(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    return Double.valueOf(s).doubleValue();
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.res.baddouble",s);
	  }
      }
    return 0;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a 
   * java.lang.BigDecimal object
   *
   * @param columnIndex  the first column is 1, the second is 2...
   * @param scale the number of digits to the right of the decimal
   * @return the column value; if the value is SQL NULL, null
   * @exception SQLException if a database access error occurs
   */
  public BigDecimal getBigDecimal(int columnIndex, int scale) throws SQLException
  {
    String s = getString(columnIndex);
    BigDecimal val;
    
    if (s != null)
      {
	try
	  {
	    val = new BigDecimal(s);
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.res.badbigdec",s);
	  }
	  try
	    {
	      return val.setScale(scale);
	    } catch (ArithmeticException e) {
		throw new PSQLException ("postgresql.res.badbigdec",s);
	    }
      }
    return null;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a Java byte array.
   *
   * <p>In normal use, the bytes represent the raw values returned by the
   * backend. However, if the column is an OID, then it is assumed to
   * refer to a Large Object, and that object is returned as a byte array.
   *
   * <p><b>Be warned</b> If the large object is huge, then you may run out
   * of memory.
   *
   * @param columnIndex the first column is 1, the second is 2, ...
   * @return the column value; if the value is SQL NULL, the result
   *	is null
   * @exception SQLException if a database access error occurs
   */
  public byte[] getBytes(int columnIndex) throws SQLException
  {
    if (columnIndex < 1 || columnIndex > fields.length)
      throw new PSQLException("postgresql.res.colrange");
    wasNullFlag = (this_row[columnIndex - 1] == null);
    
    // Handle OID's as BLOBS
    if(!wasNullFlag)
      if( fields[columnIndex - 1].getOID() == 26) {
	LargeObjectManager lom = connection.getLargeObjectAPI();
	LargeObject lob = lom.open(getInt(columnIndex));
	byte buf[] = lob.read(lob.size());
	lob.close();
	return buf;
      }
    
    return this_row[columnIndex - 1];
  }
  
  /**
   * Get the value of a column in the current row as a java.sql.Date
   * object
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return the column value; null if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public java.sql.Date getDate(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    if(s==null)
      return null;
    SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd");
    try {
      return new java.sql.Date(df.parse(s).getTime());
    } catch (ParseException e) {
      throw new PSQLException("postgresql.res.baddate",new Integer(e.getErrorOffset()),s);
    }
  }
  
  /**
   * Get the value of a column in the current row as a java.sql.Time
   * object
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return the column value; null if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public Time getTime(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    
    if (s != null)
      {
	try
	  {
	    if (s.length() != 5 && s.length() != 8)
	      throw new NumberFormatException("Wrong Length!");
	    int hr = Integer.parseInt(s.substring(0,2));
	    int min = Integer.parseInt(s.substring(3,5));
	    int sec = (s.length() == 5) ? 0 : Integer.parseInt(s.substring(6));
	    return new Time(hr, min, sec);
	  } catch (NumberFormatException e) {
	    throw new PSQLException ("postgresql.res.badtime",s);
	  }
      }
    return null;		// SQL NULL
  }
  
  /**
   * Get the value of a column in the current row as a 
   * java.sql.Timestamp object
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return the column value; null if SQL NULL
   * @exception SQLException if a database access error occurs
   */
  public Timestamp getTimestamp(int columnIndex) throws SQLException
  {
    String s = getString(columnIndex);
    if(s==null)
	return null;
    
    SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:sszzz");
    
    try {
	return new Timestamp(df.parse(s).getTime());
    } catch(ParseException e) {
	throw new PSQLException("postgresql.res.badtimestamp",new Integer(e.getErrorOffset()),s);
    }
  }
  
  /**
   * A column value can be retrieved as a stream of ASCII characters
   * and then read in chunks from the stream.  This method is 
   * particular suitable for retrieving large LONGVARCHAR values.
   * The JDBC driver will do any necessary conversion from the
   * database format into ASCII.
   *
   * <p><B>Note:</B> All the data in the returned stream must be read
   * prior to getting the value of any other column.  The next call
   * to a get method implicitly closes the stream.  Also, a stream
   * may return 0 for available() whether there is data available
   * or not.
   *
   *<p> We implement an ASCII stream as a Binary stream - we should really
   * do the data conversion, but I cannot be bothered to implement this
   * right now.
   *
   * @param columnIndex the first column is 1, the second is 2, ...
   * @return a Java InputStream that delivers the database column
   * 	value as a stream of one byte ASCII characters.  If the
   *	value is SQL NULL then the result is null
   * @exception SQLException if a database access error occurs
   * @see getBinaryStream
   */
  public InputStream getAsciiStream(int columnIndex) throws SQLException
  {
    return getBinaryStream(columnIndex);
  }
  
  /**
   * A column value can also be retrieved as a stream of Unicode
   * characters. We implement this as a binary stream.
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return a Java InputStream that delivers the database column value
   * 	as a stream of two byte Unicode characters.  If the value is
   *	SQL NULL, then the result is null
   * @exception SQLException if a database access error occurs
   * @see getAsciiStream
   * @see getBinaryStream
   */
  public InputStream getUnicodeStream(int columnIndex) throws SQLException
  {
    return getBinaryStream(columnIndex);
  }
  
  /**
   * A column value can also be retrieved as a binary strea.  This
   * method is suitable for retrieving LONGVARBINARY values.
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return a Java InputStream that delivers the database column value
   * as a stream of bytes.  If the value is SQL NULL, then the result
   * is null
   * @exception SQLException if a database access error occurs
   * @see getAsciiStream
   * @see getUnicodeStream
   */
  public InputStream getBinaryStream(int columnIndex) throws SQLException
  {
    byte b[] = getBytes(columnIndex);
    
    if (b != null)
      return new ByteArrayInputStream(b);
    return null;		// SQL NULL
  }
  
  /**
   * The following routines simply convert the columnName into
   * a columnIndex and then call the appropriate routine above.
   *
   * @param columnName is the SQL name of the column
   * @return the column value
   * @exception SQLException if a database access error occurs
   */
  public String getString(String columnName) throws SQLException
  {
    return getString(findColumn(columnName));
  }
  
  public boolean getBoolean(String columnName) throws SQLException
  {
    return getBoolean(findColumn(columnName));
  }
  
  public byte getByte(String columnName) throws SQLException
  {
    
    return getByte(findColumn(columnName));
  }
  
  public short getShort(String columnName) throws SQLException
  {
    return getShort(findColumn(columnName));
  }
  
  public int getInt(String columnName) throws SQLException
  {
    return getInt(findColumn(columnName));
  }
  
  public long getLong(String columnName) throws SQLException
  {
    return getLong(findColumn(columnName));
  }
  
  public float getFloat(String columnName) throws SQLException
  {
    return getFloat(findColumn(columnName));
  }
  
  public double getDouble(String columnName) throws SQLException
  {
    return getDouble(findColumn(columnName));
  }
  
  public BigDecimal getBigDecimal(String columnName, int scale) throws SQLException
  {
    return getBigDecimal(findColumn(columnName), scale);
  }
  
  public byte[] getBytes(String columnName) throws SQLException
  {
    return getBytes(findColumn(columnName));
  }
  
  public java.sql.Date getDate(String columnName) throws SQLException
  {
    return getDate(findColumn(columnName));
  }
  
  public Time getTime(String columnName) throws SQLException
  {
    return getTime(findColumn(columnName));
  }
  
  public Timestamp getTimestamp(String columnName) throws SQLException
  {
    return getTimestamp(findColumn(columnName));
  }
  
  public InputStream getAsciiStream(String columnName) throws SQLException
  {
    return getAsciiStream(findColumn(columnName));
  }
  
  public InputStream getUnicodeStream(String columnName) throws SQLException
  {
    return getUnicodeStream(findColumn(columnName));
  }
  
  public InputStream getBinaryStream(String columnName) throws SQLException
  {
    return getBinaryStream(findColumn(columnName));
  }
  
  /**
   * The first warning reported by calls on this ResultSet is
   * returned.  Subsequent ResultSet warnings will be chained
   * to this SQLWarning.
   *
   * <p>The warning chain is automatically cleared each time a new
   * row is read.
   *
   * <p><B>Note:</B> This warning chain only covers warnings caused by
   * ResultSet methods.  Any warnings caused by statement methods
   * (such as reading OUT parameters) will be chained on the
   * Statement object.
   *
   * @return the first SQLWarning or null;
   * @exception SQLException if a database access error occurs.
   */
  public SQLWarning getWarnings() throws SQLException
  {
    return warnings;
  }
  
  /**
   * After this call, getWarnings returns null until a new warning
   * is reported for this ResultSet
   *
   * @exception SQLException if a database access error occurs
   */
  public void clearWarnings() throws SQLException
  {
    warnings = null;
  }
  
  /**
   * Get the name of the SQL cursor used by this ResultSet
   *
   * <p>In SQL, a result table is retrieved though a cursor that is
   * named.  The current row of a result can be updated or deleted
   * using a positioned update/delete statement that references
   * the cursor name.
   *
   * <p>JDBC supports this SQL feature by providing the name of the
   * SQL cursor used by a ResultSet.  The current row of a ResulSet
   * is also the current row of this SQL cursor.
   *
   * <p><B>Note:</B> If positioned update is not supported, a SQLException
   * is thrown.
   *
   * @return the ResultSet's SQL cursor name.
   * @exception SQLException if a database access error occurs
   */
  public String getCursorName() throws SQLException
  {
    return connection.getCursorName();
  }
  
  /**
   * The numbers, types and properties of a ResultSet's columns are
   * provided by the getMetaData method
   *
   * @return a description of the ResultSet's columns
   * @exception SQLException if a database access error occurs
   */
  public java.sql.ResultSetMetaData getMetaData() throws SQLException
  {
    return new ResultSetMetaData(rows, fields);
  }
  
  /**
   * Get the value of a column in the current row as a Java object
   *
   * <p>This method will return the value of the given column as a
   * Java object.  The type of the Java object will be the default
   * Java Object type corresponding to the column's SQL type, following
   * the mapping specified in the JDBC specification.
   *
   * <p>This method may also be used to read database specific abstract
   * data types.
   *
   * @param columnIndex the first column is 1, the second is 2...
   * @return a Object holding the column value
   * @exception SQLException if a database access error occurs
   */
  public Object getObject(int columnIndex) throws SQLException
  {
    Field field;
    
    if (columnIndex < 1 || columnIndex > fields.length)
      throw new PSQLException("postgresql.res.colrange");
    field = fields[columnIndex - 1];
    
    // some fields can be null, mainly from those returned by MetaData methods
    if(field==null) {
      wasNullFlag=true;
      return null;
    }
    
    switch (field.getSQLType())
      {
      case Types.BIT:
	return new Boolean(getBoolean(columnIndex));
      case Types.SMALLINT:
	return new Integer(getInt(columnIndex));
      case Types.INTEGER:
	return new Integer(getInt(columnIndex));
      case Types.BIGINT:
	return new Long(getLong(columnIndex));
      case Types.NUMERIC:
	return getBigDecimal(columnIndex, 0);
      case Types.REAL:
	return new Float(getFloat(columnIndex));
      case Types.DOUBLE:
	return new Double(getDouble(columnIndex));
      case Types.CHAR:
      case Types.VARCHAR:
	return getString(columnIndex);
      case Types.DATE:
	return getDate(columnIndex);
      case Types.TIME:
	return getTime(columnIndex);
      case Types.TIMESTAMP:
	return getTimestamp(columnIndex);
      default:
	return connection.getObject(field.getTypeName(), getString(columnIndex));
      }
  }
  
  /**
   * Get the value of a column in the current row as a Java object
   *
   *<p> This method will return the value of the given column as a
   * Java object.  The type of the Java object will be the default
   * Java Object type corresponding to the column's SQL type, following
   * the mapping specified in the JDBC specification.
   *
   * <p>This method may also be used to read database specific abstract
   * data types.
   *
   * @param columnName is the SQL name of the column
   * @return a Object holding the column value
   * @exception SQLException if a database access error occurs
   */
  public Object getObject(String columnName) throws SQLException
  {
    return getObject(findColumn(columnName));
  }
  
  /**
   * Map a ResultSet column name to a ResultSet column index
   *
   * @param columnName the name of the column
   * @return the column index
   * @exception SQLException if a database access error occurs
   */
  public int findColumn(String columnName) throws SQLException
  {
    int i;
    
    for (i = 0 ; i < fields.length; ++i)
      if (fields[i].name.equalsIgnoreCase(columnName))
	return (i+1);
    throw new PSQLException ("postgresql.res.colname",columnName);
  }
}

