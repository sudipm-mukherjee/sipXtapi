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
package org.sipfoundry.sipxconfig.admin.dialplan.config;

import org.dom4j.Element;

/**
 * FullTransform
 */
public class FullTransform extends Transform {
    private String m_user;
    private String m_host;
    private String[] m_urlParams;
    private String[] m_fieldParams;
    private String[] m_headerParams;

    protected void addChildren(Element transform) {
        if (null != m_user) {
            Element user = transform.addElement("user");
            user.setText(m_user);
        }
        if (null != m_host) {
            Element host = transform.addElement("host");
            host.setText(m_host);

        }
        addParams(transform, "urlparams", m_urlParams);
        addParams(transform, "fieldparams", m_fieldParams);
        addParams(transform, "headerparams", m_headerParams);

    }

    private void addParams(Element transform, String name, String[] params) {
        if (null == params) {
            return;
        }
        for (int i = 0; i < params.length; i++) {
            String param = params[i];
            Element element = transform.addElement(name);
            element.setText(param);
        }
    }

    public String[] getFieldParams() {
        return m_fieldParams;
    }

    public void setFieldParams(String[] fieldParams) {
        m_fieldParams = fieldParams;
    }

    public String[] getHeaderParams() {
        return m_headerParams;
    }

    public void setHeaderParams(String[] headerParams) {
        m_headerParams = headerParams;
    }

    public String getHost() {
        return m_host;
    }

    public void setHost(String host) {
        m_host = host;
    }

    public String[] getUrlParams() {
        return m_urlParams;
    }

    public void setUrlParams(String[] urlParams) {
        m_urlParams = urlParams;
    }

    public String getUser() {
        return m_user;
    }

    public void setUser(String user) {
        m_user = user;
    }
}
