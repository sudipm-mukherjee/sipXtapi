/*
 * 
 * 
 * Copyright (C) 2006 SIPfoundry Inc.
 * Licensed by SIPfoundry under the LGPL license.
 * 
 * Copyright (C) 2006 Pingtel Corp.
 * Licensed to SIPfoundry under a Contributor Agreement.
 * 
 * $
 */
package org.sipfoundry.sipxconfig.phone;

import junit.framework.Test;
import junit.framework.TestSuite;

import org.sipfoundry.sipxconfig.TestHelper;
import org.sipfoundry.sipxconfig.test.PerformanceTestCase;

import com.clarkware.junitperf.TimedTest;

public class PhoneCreateTestPerformance  extends PerformanceTestCase {
    
    private PhoneContext m_context;
    
    public PhoneCreateTestPerformance(String singleTestMethodName) {
        super(singleTestMethodName);
    }
    
    public static Test suite() {

        TestSuite suite = new TestSuite();

        suite.addTest(new TimedTest(new PhoneCreateTestPerformance("testCreate"), getTolerance(2500)));
        
        return suite;
    }
    
    protected void setUpUnTimed() {
        m_context = (PhoneContext) TestHelper.getApplicationContext().getBean(
                PhoneContext.CONTEXT_BEAN_NAME);
        m_context.clear();
    }
    
    public void testCreate() {
        seedPhones(m_context, 100);
    }
    
    static void seedPhones(PhoneContext context, int count) {
        for (int i = 0; i < count; i++) {
            Phone p = context.newPhone(Phone.MODEL);
            p.setSerialNumber("ff00" + String.valueOf(i));
            p.addLine(p.createLine());
            context.storePhone(p);
        }
    }    
}
