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
package org.sipfoundry.sipxconfig.setting.type;

import org.apache.commons.lang.StringUtils;

/**
 * Special type of setting used for upload-able file.
 */
public class FileSetting implements SettingType {
    private boolean m_required;
    private boolean m_variable;

    /** Directory in which downloaded files are stored */
    private String m_directory;

    /** Mime content type */
    private String m_contentType = "audio/x-wav";

    public boolean isRequired() {
        return m_required;
    }

    public void setRequired(boolean required) {
        m_required = required;
    }
    
    public boolean isVariable() {
        return m_variable;
    }
    
    public void setVariable(boolean variable) {
        m_variable = variable;
    }

    public String getDirectory() {
        return m_directory;
    }

    public void setDirectory(String directory) {
        m_directory = directory;
    }

    public void setContentType(String contentType) {
        m_contentType = contentType;
    }

    public String getContentType() {
        return m_contentType;
    }

    public String getName() {
        return "file";
    }

    public Object convertToTypedValue(Object value) {
        return value;
    }
    
    public String convertToStringValue(Object value) {
        if (value == null) {
            return null;
        }
        return value.toString();
    }    

    public String getLabel(Object value) {
        return convertToStringValue(value);
    }    
}
