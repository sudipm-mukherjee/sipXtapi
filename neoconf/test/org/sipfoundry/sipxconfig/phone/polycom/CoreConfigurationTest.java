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
package org.sipfoundry.sipxconfig.phone.polycom;

import java.io.CharArrayReader;
import java.io.CharArrayWriter;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.Reader;

import org.custommonkey.xmlunit.Diff;
import org.custommonkey.xmlunit.XMLTestCase;
import org.custommonkey.xmlunit.XMLUnit;
import org.sipfoundry.sipxconfig.setting.Setting;
import org.sipfoundry.sipxconfig.setting.SettingGroup;


public class CoreConfigurationTest extends XMLTestCase {
    
    public void setUp() {
        XMLUnit.setIgnoreWhitespace(true);
    }

    public void testGenerateProfile() throws Exception {
        PolycomTestHelper helper = PolycomTestHelper.plainEndpointSeed();        

        SettingGroup settings = helper.endpoint[0].getSettings(helper.phone[0]);
        Setting call = settings.getSetting("call"); 
        Setting rejectDnd = call.getSetting("rejectBusyOnDnd");
        rejectDnd.setValue("0");
        
        CoreConfiguration cfg = new CoreConfiguration(
            helper.phone[0], helper.endpoint[0]);
        
        cfg.setTemplate(helper.phone[0].getConfig().getCoreTemplate());
        CharArrayWriter out = new CharArrayWriter();
        cfg.generateProfile(out);       
        
        InputStream expectedPhoneStream = getClass().getResourceAsStream("cfgdata/expected-ipmid.cfg");
        Reader expectedXml = new InputStreamReader(expectedPhoneStream);            
        Reader generatedXml = new CharArrayReader(out.toCharArray());

        // helpful debug
        System.out.println(new String(out.toCharArray()));

        Diff phoneDiff = new Diff(expectedXml, generatedXml);
        assertXMLEqual(phoneDiff, true);
        expectedPhoneStream.close();        
    }
}
