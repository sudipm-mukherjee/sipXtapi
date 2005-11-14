/*
 * 
 * 
 * Copyright (C) 2005 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2005 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.search;

import java.io.File;
import java.io.IOException;

import org.apache.lucene.analysis.Analyzer;
import org.apache.lucene.index.IndexReader;
import org.apache.lucene.index.IndexWriter;
import org.apache.lucene.search.IndexSearcher;
import org.apache.lucene.store.Directory;
import org.apache.lucene.store.FSDirectory;

/**
 * Naive implementation, always return FS directory, do not try to cache or optimize anything,
 * recreate if it does not exist
 * 
 */
public class SimpleIndexSource implements IndexSource {
    private File m_indexDirectory;

    private boolean m_createDirectory;

    private boolean m_createIndex;

    private Analyzer m_analyzer;

    private int m_maxFieldLength = IndexWriter.DEFAULT_MAX_FIELD_LENGTH;

    private int m_minMergeDocs = IndexWriter.DEFAULT_MIN_MERGE_DOCS;

    private long m_writeLockTimeout = IndexWriter.WRITE_LOCK_TIMEOUT;

    private Directory getDirectory() {
        try {
            Directory directory = createDirectory(m_indexDirectory, m_createDirectory);
            m_createDirectory = false;
            return directory;
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
    }

    /**
     * Overwrite to create different directory
     * 
     * @throws IOException
     */
    protected Directory createDirectory(File file, boolean createDirectory) throws IOException {
        return FSDirectory.getDirectory(file, createDirectory);
    }

    public void setIndexDirectoryName(String indexDirectoryName) {
        m_indexDirectory = new File(indexDirectoryName);
        m_createDirectory = !m_indexDirectory.exists();
        m_createIndex = m_createDirectory;
    }

    public void setAnalyzer(Analyzer analyzer) {
        m_analyzer = analyzer;
    }

    public void setMinMergeDocs(int minMergeDocs) {
        m_minMergeDocs = minMergeDocs;
    }

    public void setMaxFieldLength(int maxFieldLength) {
        m_maxFieldLength = maxFieldLength;
    }

    public void setWriteLockTimeout(long writeLockTimeout) {
        m_writeLockTimeout = writeLockTimeout;
    }

    public IndexReader getReader() throws IOException {
        ensureIndexExists();
        return IndexReader.open(getDirectory());
    }

    public IndexWriter getWriter(boolean createNew) throws IOException {
        IndexWriter writer = new IndexWriter(getDirectory(), m_analyzer, createNew
                || m_createIndex);
        writer.maxFieldLength = m_maxFieldLength;
        writer.minMergeDocs = m_minMergeDocs;
        writer.WRITE_LOCK_TIMEOUT = m_writeLockTimeout;
        m_createIndex = false;
        return writer;
    }

    public IndexSearcher getSearcher() throws IOException {
        ensureIndexExists();
        return new IndexSearcher(getDirectory());
    }

    private void ensureIndexExists() throws IOException {
        if (m_createIndex) {
            IndexWriter writer = getWriter(false);
            writer.close();
        }
    }
}
