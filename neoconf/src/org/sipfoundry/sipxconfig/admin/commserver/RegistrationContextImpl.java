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
package org.sipfoundry.sipxconfig.admin.commserver;

import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.net.URL;
import java.text.MessageFormat;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.collections.map.LinkedMap;
import org.apache.commons.digester.Digester;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.ImdbXmlHelper;
import org.sipfoundry.sipxconfig.admin.commserver.imdb.RegistrationItem;
import org.xml.sax.SAXException;

public class RegistrationContextImpl implements RegistrationContext {
    public static final Log LOG = LogFactory.getLog(RegistrationContextImpl.class);

    private static final String REGISTRATION_URL = "https://{0}:{1}/sipdb/registration.xml";

    private String m_server;

    private String m_port;

    /**
     * @see org.sipfoundry.sipxconfig.admin.commserver.RegistrationContext#getRegistrations()
     */
    public List getRegistrations() {
        try {
            URL url = new URL(getUrl());
            InputStream is = url.openStream();
            return getRegistrations(is);
        } catch (FileNotFoundException e) {
            // we are handling this separately - server returns FileNotFound even if everything is
            // OK but we have no registrations present
            LOG.info("No registrations found on the server" + e.getMessage());
            return Collections.EMPTY_LIST;
        } catch (IOException e) {
            LOG.error("Retrieving active registrations failed", e);
            return Collections.EMPTY_LIST;
        } catch (SAXException e) {
            throw new RuntimeException(e);
        }
    }

    List getRegistrations(InputStream is) throws IOException, SAXException {
        Digester digester = ImdbXmlHelper.configureDigester(RegistrationItem.class);
        return (List) digester.parse(is);
    }

    /**
     * Remove multiple registrations for a single contact
     * 
     * @param registrations
     * @return registration list without duplicated
     */
    List cleanRegistrations(List registrations) {
        LinkedMap contact2registration = new LinkedMap(registrations.size());
        for (Iterator iter = registrations.iterator(); iter.hasNext();) {
            RegistrationItem ri = (RegistrationItem) iter.next();
            String contact = ri.getContact();
            RegistrationItem riOld = (RegistrationItem) contact2registration.get(contact);
            // replace older registrations
            if (riOld == null || ri.compareTo(riOld) > 0) {
                contact2registration.put(contact, ri);
            }
        }
        return new ArrayList(contact2registration.values());
    }

    String getUrl() {
        Object[] params = {
            m_server, m_port
        };
        return MessageFormat.format(REGISTRATION_URL, params);
    }

    public void setPort(String port) {
        m_port = port;
    }

    public void setServer(String server) {
        m_server = server;
    }
}
