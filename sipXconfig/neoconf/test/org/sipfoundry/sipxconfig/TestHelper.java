/*
 * 
 * 
 * Copyright (C) 2004 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2004 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig;

import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.sql.Connection;
import java.sql.SQLException;
import java.util.Properties;

import javax.sql.DataSource;

import junit.framework.TestCase;

import org.apache.commons.io.IOUtils;
import org.apache.velocity.app.VelocityEngine;
import org.dbunit.database.DatabaseConfig;
import org.dbunit.database.DatabaseConnection;
import org.dbunit.database.IDatabaseConnection;
import org.dbunit.dataset.IDataSet;
import org.dbunit.dataset.ReplacementDataSet;
import org.dbunit.dataset.xml.FlatXmlDataSet;
import org.dbunit.dataset.xml.XmlDataSet;
import org.dbunit.operation.DatabaseOperation;
import org.easymock.EasyMock;
import org.easymock.IMocksControl;
import org.sipfoundry.sipxconfig.common.TestUtil;
import org.sipfoundry.sipxconfig.domain.Domain;
import org.sipfoundry.sipxconfig.domain.DomainManager;
import org.sipfoundry.sipxconfig.setting.ModelBuilder;
import org.sipfoundry.sipxconfig.setting.ModelFilesContext;
import org.sipfoundry.sipxconfig.setting.ModelFilesContextImpl;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingSet;
import org.sipfoundry.sipxconfig.setting.XmlModelBuilder;
import org.springframework.beans.factory.access.BeanFactoryLocator;
import org.springframework.beans.factory.access.BeanFactoryReference;
import org.springframework.context.ApplicationContext;
import org.springframework.context.access.ContextSingletonBeanFactoryLocator;
import org.springframework.dao.DataIntegrityViolationException;

/**
 * TestHelper: used for unit tests that need Spring instantiated
 */
public final class TestHelper {

    private static Properties s_sysDirProps;

    private static ApplicationContext s_appContext;

    private static DatabaseConnection s_dbunitConnection;

    static {
        // The default XML parser (Apache Crimson) cannot resolve relative DTDs, google to find
        // the bug.
        System.setProperty("org.xml.sax.driver", "org.apache.xerces.parsers.SAXParser");

        // The method SAXParserFactory.newInstance is using this system property to find the
        // parser factory.
        // So we have to set this property instead of, or in addition to, the one above.
        // Fixes XCF-537: jdk dependency in DB unit tests, fails with jdk 1.4.2, works with jdk
        // 1.5.
        System.setProperty("javax.xml.parsers.SAXParserFactory",
                "org.apache.xerces.jaxp.SAXParserFactoryImpl");
        
        // Activate log configuration on test/log4j.properties 
        // System.setProperty("org.apache.commons.logging.Log", 
        //        "org.apache.commons.logging.impl.Log4JLogger");                
    }

    public static ApplicationContext getApplicationContext() {
        if (s_appContext == null) {
            getSysDirProperties();
            BeanFactoryLocator bfl = ContextSingletonBeanFactoryLocator.getInstance();
            BeanFactoryReference bfr = bfl.useBeanFactory("servicelayer-context");
            s_appContext = (ApplicationContext) bfr.getFactory();
        }

        return s_appContext;
    }
    
    public static DomainManager getTestDomainManager(String domain) {
        Domain exampleDomain = new Domain(domain);
        IMocksControl domainManagerControl = EasyMock.createControl();
        DomainManager domainManager = domainManagerControl.createMock(DomainManager.class);
        domainManager.getDomain();
        domainManagerControl.andReturn(exampleDomain).anyTimes();
        domainManagerControl.replay();        
        return domainManager;
    }

    public static ModelFilesContext getModelFilesContext() {
        ModelFilesContextImpl mfc = new ModelFilesContextImpl();
        String sysdir = getSettingModelContextRoot();
        mfc.setConfigDirectory(sysdir);
        XmlModelBuilder builder = new XmlModelBuilder(sysdir);
        mfc.setModelBuilder(builder);
        return mfc;
    }

    public static String getSettingModelContextRoot() {
        String sysdir = getSysDirProperties().getProperty("sysdir.etc");
        return sysdir;
    }

    public static Setting loadSettings(String path) {
        Setting settings = getModelFilesContext().loadModelFile(path);
        return settings;
    }
    
    public static Setting loadSettings(InputStream in) {        
        ModelBuilder builder = new XmlModelBuilder("etc");
        SettingSet root;
        try {
            root = builder.buildModel(in, null);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        return root;
    }

    public static String getClasspathDirectory() {
        return TestUtil.getClasspathDirectory(TestHelper.class);
    }

    public static VelocityEngine getVelocityEngine() {

        try {
            Properties sysdir = getSysDirProperties();

            String etcDir = sysdir.getProperty("sysdir.etc");

            VelocityEngine engine = new VelocityEngine();
            engine.setProperty("resource.loader", "file");
            engine.setProperty("file.resource.loader.path", etcDir);
            engine.init();

            return engine;
        } catch (Exception e) {
            throw new RuntimeException(e);
        }
    }

    public static String getTestDirectory() {
        return TestUtil.getTestOutputDirectory("neoconf");
    }

    public static Properties getSysDirProperties() {
        if (s_sysDirProps == null) {
            String classpathDirectory = getClasspathDirectory();
            initSysDirProperties(classpathDirectory);
        }
        return s_sysDirProps;
    }

    public static void initSysDirProperties(String dir) {
        String etcDir = TestUtil.getProjectDirectory() + "/etc";
        String outDir = getTestDirectory();
        s_sysDirProps = new Properties();
        TestUtil.setSysDirProperties(s_sysDirProps, etcDir, outDir);
        TestUtil.saveSysDirProperties(s_sysDirProps, dir);
    }

    public static IDatabaseConnection getConnection() throws Exception {
        if (s_dbunitConnection != null) {
            return s_dbunitConnection;
        }
        Class.forName("com.p6spy.engine.spy.P6SpyDriver");

        DataSource ds = (DataSource) getApplicationContext().getBean("dataSource");
        Connection jdbcConnection = ds.getConnection();
        s_dbunitConnection = new DatabaseConnection(jdbcConnection);
        DatabaseConfig config = s_dbunitConnection.getConfig();
        config.setFeature("http://www.dbunit.org/features/batchedStatements", true);

        return s_dbunitConnection;
    }

    public static void closeConnection() throws SQLException {
        if (s_dbunitConnection != null && !s_dbunitConnection.getConnection().isClosed()) {
            s_dbunitConnection.close();
            s_dbunitConnection = null;
        }
    }

    public static IDataSet loadDataSet(String fileResource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(fileResource);
        return new XmlDataSet(datasetStream);
    }

    public static IDataSet loadDataSetFlat(String resource) throws Exception {
        InputStream datasetStream = TestHelper.class.getResourceAsStream(resource);
        // we do not want to use metadata since DBTestUnit resolves relative DTDs incorrectly
        // we are checking XML validity in separate Ant tasks (test-dataset)
        return new FlatXmlDataSet(datasetStream, false);
    }
    
    public static ReplacementDataSet loadReplaceableDataSetFlat(String fileResource) throws Exception {
    		IDataSet ds = loadDataSetFlat(fileResource);
    		ReplacementDataSet relaceable = new ReplacementDataSet(ds);
    		relaceable.addReplacementObject("[null]", null);
    		return relaceable;    		
    }

    public static void cleanInsert(String resource) throws Exception {
        try {
            DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSet(resource));
        } catch (SQLException e) {
            throw e.getNextException();
        }
    }

    public static void cleanInsertFlat(String resource) throws Exception {
        DatabaseOperation.CLEAN_INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void insertFlat(String resource) throws Exception {
        DatabaseOperation.INSERT.execute(getConnection(), loadDataSetFlat(resource));
    }

    public static void update(String resource) throws Exception {
        DatabaseOperation.UPDATE.execute(getConnection(), loadDataSet(resource));
    }

    /**
     * Special TestCase class that catches prints additional info for SQL Exceptions errors that
     * may happed during setUp, testXXX and tearDown.
     * 
     * Alternatively we could just throw e.getNextException, but we may want to preserve the
     * original exception.
     */
    public static class TestCaseDb extends TestCase {
        public void runBare() throws Throwable {
            try {
                super.runBare();
            } catch (SQLException e) {
                dumpSqlExceptionMessages(e);
                throw e;
            } catch (DataIntegrityViolationException e) {
                if (e.getCause() instanceof SQLException) {
                    dumpSqlExceptionMessages((SQLException) e.getCause());
                }
                throw e;
            }
        }

        private void dumpSqlExceptionMessages(SQLException e) {
            for (SQLException next = e; next != null; next = next.getNextException()) {
                System.err.println(next.getMessage());
            }
        }
    }

    /**
     * Use in test to create copy of example files to be changed by test methods.
     * 
     * @param from input stream
     * @param dir destination directory
     * @param filename destination file name
     * @throws IOException
     */
    public static final void copyStreamToDirectory(InputStream from, String dir, String filename)
            throws IOException {
        FileOutputStream to = new FileOutputStream(new File(dir, filename));
        IOUtils.copy(from, to);
        IOUtils.closeQuietly(to);
        IOUtils.closeQuietly(from);
    }
}
