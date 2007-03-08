//
// Copyright (C) 2004-2006 SIPfoundry Inc.
// Licensed by SIPfoundry under the LGPL license.
//
// Copyright (C) 2004-2006 Pingtel Corp.  All rights reserved.
// Licensed to SIPfoundry under a Contributor Agreement.
//
// $$
///////////////////////////////////////////////////////////////////////////////

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/TestCase.h>
#include <sipxunit/TestUtilities.h>

#include <string.h>
#include <net/Url.h>
#include <net/HttpMessage.h>
#include <net/NetMd5Codec.h>
#include <utl/UtlTokenizer.h>

#include "os/OsTimeLog.h"

#define MISSING_PARAM  "---missing---"

#define ASSERT_ARRAY_MESSAGE(message, expected, actual) \
  UrlTest::assertArrayMessage((expected),(actual), \
      CPPUNIT_SOURCELINE(), (message))

class UrlTest : public CppUnit::TestCase
{
    CPPUNIT_TEST_SUITE(UrlTest);
    CPPUNIT_TEST(testFileBasic);
    CPPUNIT_TEST(testFileWithPortAndPath);
    CPPUNIT_TEST(testHttpBasic);
    CPPUNIT_TEST(testHttpWithPortAndPath);
    CPPUNIT_TEST(testHttpWithQuery);
    CPPUNIT_TEST(testHttpWithQueryNameAddr);
    CPPUNIT_TEST(testHttpWithQueryAddrSpec);
    CPPUNIT_TEST(testSipBasic); 
    CPPUNIT_TEST(testSipBasicWithPort);
    CPPUNIT_TEST(testIpBasicWithBrackets);
    CPPUNIT_TEST(testSemiHeaderParam);
    CPPUNIT_TEST(testSipAdvanced); 
    CPPUNIT_TEST(testSipComplexUser);
    CPPUNIT_TEST(testLongHostname);
    CPPUNIT_TEST(testSipParameters);
    CPPUNIT_TEST(testFullSip);
    CPPUNIT_TEST(testQuotedName);
    CPPUNIT_TEST(testFancyNames);
    CPPUNIT_TEST(testEncoded);
    CPPUNIT_TEST(testNoFieldParams);
    CPPUNIT_TEST(testNoHeaderParams);
    CPPUNIT_TEST(testCorrection);
    CPPUNIT_TEST(testIpAddressOnly);
    CPPUNIT_TEST(testMissingAngles);
    CPPUNIT_TEST(testNoAngleParam);
    CPPUNIT_TEST(testHostAddressOnly);
    CPPUNIT_TEST(testHostAndPort);
    CPPUNIT_TEST(testIPv6Host);
    CPPUNIT_TEST(testBogusPort);
    CPPUNIT_TEST(testNoBracketUrlWithAllParamsWithVaryingSpace);
    CPPUNIT_TEST(testConstruction);
    CPPUNIT_TEST(testHttpConstruction);
    CPPUNIT_TEST(testComplexConstruction);
    CPPUNIT_TEST(testAddAttributesToExisting);
    CPPUNIT_TEST(testChangeValues);
    CPPUNIT_TEST(testRemoveAttributes);
    CPPUNIT_TEST(testRemoveUrlParameterCase);
    CPPUNIT_TEST(testRemoveFieldParameterCase);
    CPPUNIT_TEST(testRemoveHeaderParameterCase);
    CPPUNIT_TEST(testRemoveAllTypesOfAttributes);
    CPPUNIT_TEST(testRemoveAngleBrackets);
    CPPUNIT_TEST(testReset);
    CPPUNIT_TEST(testAssignment);
    CPPUNIT_TEST(testGetAllParameters);
    CPPUNIT_TEST(testGetDuplicateNamedParameters);
    CPPUNIT_TEST(testGetOnlyUrlParameters);
    CPPUNIT_TEST(testGetOnlyHeaderParameters);
    CPPUNIT_TEST(testGetOnlyFieldParameters);
    CPPUNIT_TEST(testGetUrlParameterCase);
    CPPUNIT_TEST(testGetFieldParameterCase);
    CPPUNIT_TEST(testGetHeaderParameterCase);
    CPPUNIT_TEST(testIsDigitString);
    CPPUNIT_TEST(testIsUserHostPortEqualExact);
    CPPUNIT_TEST(testIsUserHostPortVaryCapitalization);
    CPPUNIT_TEST(testIsUserHostPortNoPort);
    CPPUNIT_TEST(testIsUserHostPortNoMatch);
    CPPUNIT_TEST(testIsUserHostPortPorts);
    CPPUNIT_TEST(testToString);
    CPPUNIT_TEST(testFromString);
    CPPUNIT_TEST(testGetIdentity);
    CPPUNIT_TEST(testNormalUri);
    CPPUNIT_TEST(testBigUriDisplayName);
    CPPUNIT_TEST(testBigUriQuotedName);
    CPPUNIT_TEST(testBigUriScheme);
    CPPUNIT_TEST(testBigUriUser);
    CPPUNIT_TEST(testBigUriNoSchemeUser);
    CPPUNIT_TEST(testBigUriHost);
    CPPUNIT_TEST_SUITE_END();

private:

    UtlString *assertValue;

    static const size_t BIG_SIZE;
    UtlString bigtoken; // bigtoken = BIG_SIZE characters to be used in constructing strings

public:

    void setUp()
    {
        assertValue = new UtlString();

        CPPUNIT_ASSERT(bigtoken.capacity(BIG_SIZE) >= BIG_SIZE);
        while (bigtoken.length() < BIG_SIZE)
        {
           bigtoken.append("a123456789"); // use leading a to match name syntax
        }
    }

    void tearDown()
    {
        delete assertValue;
    }

    void testFileBasic()
    {
        const char* szUrl =  "file://www.sipfoundry.org/dddd/ffff.txt";        
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "simple file url : %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "www.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "file", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
    }

    void testFileWithPortAndPath()
    {
        const char* szUrl = "file://server:8080/dddd/ffff.txt";

        char msg[1024];
        sprintf(msg, "file url w/path and port : %s", szUrl);
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "file", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testHttpBasic()
    {
        const char* szUrl =  "http://www.sipfoundry.org";        

        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "simple http url : %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "www.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());
    }

   void testHttpWithPortAndPath()
      {
         const char* szUrl = "http://server:8080/dddd/ffff.txt";
         char msg[1024];
         sprintf(msg, "url w/path and port : %s", szUrl);
         Url url(szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url,TRUE));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
      }

   void testHttpWithQuery()
      {
         const char* szUrl = "http://server:8080/dddd/ffff.txt?p1=v1&p2=v2";
         char msg[1024];
         sprintf(msg, "url w/path and port : %s", szUrl);
         Url url(szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, "server", getHostAddress(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "http", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt", getPath(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "/dddd/ffff.txt?p1=v1&p2=v2", getPath(url,TRUE));

         CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(msg, "v1", getHeaderParam("p1",url));
         ASSERT_STR_EQUAL_MESSAGE(msg, "v2", getHeaderParam("p2",url));

         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
      }

   void testHttpWithQueryNameAddr()
      {
         const char* szUrl = "https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox=111&from=%22Dale+Worley%22%3Csip%3A173%40pingtel.com%3E%3Btag%253D3c11304";
         Url url(szUrl);
         char msg[1024];
         sprintf(msg, "http url with query (name-addr) : %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));

         const char* szUrl2 = "https://localhost/mediaserver.cgi?foo=bar";

         Url url2(szUrl2);
         sprintf(msg, "http url with query (name-addr) : %s", szUrl2);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl2, toString(url2));
      }

   void testHttpWithQueryAddrSpec()
      {
         const char* szUrl = "https://localhost:8091/cgi-bin/voicemail/mediaserver.cgi?action=deposit&mailbox=111&from=%22Dale+Worley%22%3Csip%3A173%40pingtel.com%3E%3Btag%253D3c11304";
#ifdef _WIN32
//         KNOWN_FATAL_BUG("Returned path separator is wrong under Win32", "XSL-74");
#endif   
         Url url(szUrl, TRUE);
         char msg[1024];
         sprintf(msg, "http url with query (addr-spec) : %s", szUrl);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));

         const char* szUrl2 = "https://localhost/mediaserver.cgi?foo=bar";

         Url url2(szUrl2, TRUE);
         sprintf(msg, "http url with query (addr-spec) : %s", szUrl2);
         ASSERT_STR_EQUAL_MESSAGE(msg, szUrl2, toString(url2));
      }


    void testSipBasic()
    {
        const char* szUrl = "sip:rschaaf@10.1.1.89";

        char msg[1024];
        sprintf(msg, "sip url with ip address: %s", szUrl);
        Url url(szUrl); 
        ASSERT_STR_EQUAL_MESSAGE(msg, "10.1.1.89", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testSipBasicWithPort() 
    {
        const char* szUrl = "sip:fsmith@sipfoundry.org:5555";

        char msg[1024];
        sprintf(msg, "sip url with port: %s", szUrl);
        Url url(szUrl); 
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 5555, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, getUri(url));
    }

    void testIpBasicWithBrackets()
    {
        const char* szUrl = "<sip:rschaaf@sipfoundry.org>";

        char msg[1024];
        sprintf(msg, "url sip address: %s", szUrl);
        Url url(szUrl); 
        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "rschaaf", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:rschaaf@sipfoundry.org", toString(url));
        url.includeAngleBrackets();

        ASSERT_STR_EQUAL_MESSAGE(msg, "<sip:rschaaf@sipfoundry.org>", toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:rschaaf@sipfoundry.org", getUri(url));
    }

    void testSipAdvanced()
    {
        const char* szUrl = "Rich Schaaf<sip:sip.tel.sipfoundry.org:8080>" ;

        char msg[1024];
        sprintf(msg, "advanced bracketed sip address: %s", szUrl);
        Url url(szUrl); 

        ASSERT_STR_EQUAL_MESSAGE(msg, "sip.tel.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "Rich Schaaf", getDisplayName(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 8080, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:sip.tel.sipfoundry.org:8080", getUri(url));
    }

    void testSipComplexUser()
    {
        const char* szUrl = "Raghu Venkataramana<sip:user-tester.my/place?"
            "&yourplace@sipfoundry.org>";

        char msg[1024];
        sprintf(msg, "complex user sip address: %s", szUrl);
        Url url(szUrl);

        ASSERT_STR_EQUAL_MESSAGE(msg, "sipfoundry.org", 
            getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "user-tester.my/place?&yourplace", 
            getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:user-tester.my/place?&yourplace@sipfoundry.org",
                                 getUri(url));
    }


    void testLongHostname()
    {
        const char* szUrl =
           "Raghu Venkataramana<sip:125@testing.stage.inhouse.sipfoundry.org>" ; 

        char msg[1024];
        sprintf(msg, "long hostname sip address: %s", szUrl);
        Url url(szUrl);

        ASSERT_STR_EQUAL_MESSAGE(msg, "testing.stage.inhouse.sipfoundry.org", 
            getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "125", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, PORT_NONE, url.getHostPort());

        ASSERT_STR_EQUAL_MESSAGE(msg, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "sip:125@testing.stage.inhouse.sipfoundry.org", getUri(url));
    }

    void testSipParameters()
    {
        const char *szUrl = "<sip:username@10.1.1.225:555;tag=xxxxx;transport=TCP;"
            "msgId=4?call-Id=call2&cseq=2+INVITE>;fieldParam1=1234;fieldParam2=2345";

        const char *szUrlCorrected = "<sip:username@10.1.1.225:555;tag=xxxxx;"
            "transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;fieldParam1=1234;"
            "fieldParam2=2345";

        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "10.1.1.225", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "username", getUserId(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 555, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "xxxxx", getParam("tag", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "TCP", getParam("transport", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "4", getParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "call2", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "2 INVITE", getHeaderParam("cseq", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "1234", getFieldParam("fieldParam1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "2345", getFieldParam("fieldParam2", url));
    }

    void testFullSip()
    {
        const char *szUrl = "Display Name<sip:uname@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipserver", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip", getUrlType(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "uname", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "Display Name", getDisplayName(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 555, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "xxxxx", getParam("tag", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "TCP", getParam("transport", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "4", getParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "call2", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "2 INVITE", getHeaderParam("cseq", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "1234", getFieldParam("fieldParam1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "2345", getFieldParam("fieldParam2", url));

        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(
           szUrl,
           "sip:uname@sipserver:555;tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE",
           getUri(url)
                                 );
    }

   void testQuotedName()
      {
         const char *szUrl = "\"Display \\\"Name\"<sip:easy@sipserver>";
         Url url(szUrl);
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "\"Display \\\"Name\"", getDisplayName(url));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip", getUrlType(url));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "easy", getUserId(url));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipserver", getHostAddress(url));
         CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, PORT_NONE, url.getHostPort());

         ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl, toString(url));
         ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:easy@sipserver", getUri(url));
      }

    void testFancyNames()
    {
        const char *szUrl = "\"(Display \\\"< @ Name)\"  <sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
        const char *szUrl_corrected = "\"(Display \\\"< @ Name)\"<sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
            "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE>;"
            "fieldParam1=1234;fieldParam2=2345";
	Url url(szUrl);
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "\"(Display \\\"< @ Name)\"", getDisplayName(url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip", getUrlType(url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-", getUserId(url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipserver", getHostAddress(url));
	CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 555, url.getHostPort());
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "xxxxx", getParam("tag", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "TCP", getParam("transport", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "4", getParam("msgId", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "call2", getHeaderParam("call-Id", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "2 INVITE", getHeaderParam("cseq", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "1234", getFieldParam("fieldParam1", url));
	ASSERT_STR_EQUAL_MESSAGE(szUrl, "2345", getFieldParam("fieldParam2", url));

	ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl_corrected, toString(url));
	ASSERT_STR_EQUAL_MESSAGE(
	   szUrl,
	   "sip:?$,;silly/user+(name)_&=.punc%2d!bing*bang~'-@sipserver:555;"
	   "tag=xxxxx;transport=TCP;msgId=4?call-Id=call2&cseq=2+INVITE",
	   getUri(url));
    }

    void testEncoded()
    {
        const char *szUrl = "D Name<sip:autoattendant@sipfoundry.org:5100;"
            "play=http%3A%2F%2Flocalhost%3A8090%2Fsipx-cgi%2Fmediaserver.cgi"
            "%3Faction%3Dautoattendant?hp1=hval1>;fp1=fval1";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "autoattendant", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "autoattendant@sipfoundry.org:5100", 
            getIdentity(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 5100, url.getHostPort());
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "http://localhost:8090/sipx-cgi/mediaserver.cgi?"
            "action=autoattendant", getParam("play", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "hval1", getHeaderParam("hp1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "fval1", getFieldParam("fp1", url));

        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl,
                                 "sip:autoattendant@sipfoundry.org:5100;"
                                 "play=http%3A%2F%2Flocalhost%3A8090%2Fsipx-cgi%2Fmediaserver.cgi"
                                 "%3Faction%3Dautoattendant?hp1=hval1",
                                 getUri(url));
    }

    void testNoFieldParams()
    {
        const char *szUrl = "<sip:tester@sipfoundry.org;up1=uval1;up2=uval2?"
            "hp1=hval1&hp2=hval2>";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "uval1", getParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "uval2", getParam("up2", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "hval1", getHeaderParam("hp1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "hval2", getHeaderParam("hp2", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getFieldParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getFieldParam("hp1", url));

        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(
           szUrl,
           "sip:tester@sipfoundry.org;up1=uval1;up2=uval2?hp1=hval1&hp2=hval2",
           getUri(url)
                                 );
    }

    void testNoHeaderParams()
    {
        const char *szUrl = "Display Name<sip:tester@sipfoundry.org;up1=uval1;up2=uval2>"
            ";Fp1=fval1;Fp2=fval2";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "uval1", getParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "uval2", getParam("up2", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "fval1", getFieldParam("Fp1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "fval2", getFieldParam("Fp2", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getHeaderParam("up1", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getHeaderParam("Fp1", url));

        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrl, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl,
                                 "sip:tester@sipfoundry.org;up1=uval1;up2=uval2",
                                 getUri(url));
    }

    void testCorrection()
    {
        const char *szUrl = "Display Name <Sip:tester@sipfoundry.org>";

        const char *szUrlCorrected = "Display Name<sip:tester@sipfoundry.org>";

        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "has space and wrong capitalization in Sip: %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
    }

    void testSemiHeaderParam()
    {
        const char *withAngles          = "<sip:tester@sipfoundry.org?foo=bar;bing>";
        const char *withoutAngles        = "sip:tester@sipfoundry.org?foo=bar;bing";
        const char *withAnglesEscaped   = "<sip:tester@sipfoundry.org?foo=bar%3Bbing>";
        const char *withoutAnglesEscaped = "sip:tester@sipfoundry.org?foo=bar%3Bbing";

        Url urlHdr(withAngles);
        char msg[1024];
        sprintf(msg, "with angle brackets %s", withAngles);

        ASSERT_STR_EQUAL_MESSAGE(msg, "bar;bing", getHeaderParam("foo", urlHdr));

        ASSERT_STR_EQUAL_MESSAGE(msg, withAnglesEscaped, toString(urlHdr));
        ASSERT_STR_EQUAL_MESSAGE(msg, withoutAnglesEscaped, getUri(urlHdr));

        Url url(withoutAngles, TRUE /* parse as a request uri */ );
        sprintf(msg, "without angle brackets %s", withoutAngles);

        ASSERT_STR_EQUAL_MESSAGE(msg, "bar;bing", getHeaderParam("foo", url));

        ASSERT_STR_EQUAL_MESSAGE(msg, withAnglesEscaped, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, withoutAnglesEscaped, getUri(url));
    }

    void testMissingAngles()
    {
        const char *szUrl = "sip:tester@sipfoundry.org?foo=bar";

        const char *szUrlCorrected = "<sip:tester@sipfoundry.org?foo=bar>";

        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "needed angle brackets %s", szUrl);
        ASSERT_STR_EQUAL_MESSAGE(msg, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(msg, "bar", getHeaderParam("foo", url));
    }

    void testNoAngleParam()
    {
        const char *szUrl = "sip:tester@sipfoundry.org;foo=bar";

        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getParam("foo", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "bar", getFieldParam("foo", url));

        Url requrl(szUrl, TRUE);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "bar", getParam("foo", requrl));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, MISSING_PARAM, getFieldParam("foo", requrl));
    }


    void testIpAddressOnly()
    {
        const char *szUrl = "10.1.1.225";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:10.1.1.225", toString(url));
    }

    void testHostAddressOnly()
    {
        const char *szUrl = "somewhere.sipfoundry.org";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "somewhere.sipfoundry.org", getHostAddress(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:somewhere.sipfoundry.org", toString(url));

        const char *szUrl2 = "some-where.sipfoundry.org";
        Url url2(szUrl2);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "some-where.sipfoundry.org", getHostAddress(url2));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:some-where.sipfoundry.org", toString(url2));

        UtlString hostWithPort;
        url.getHostWithPort(hostWithPort);
        ASSERT_STR_EQUAL("somewhere.sipfoundry.org", hostWithPort.data());
    }

    void testHostAndPort()
    {
        const char *szUrl = "somewhere.sipfoundry.org:333";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "somewhere.sipfoundry.org", getHostAddress(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 333, url.getHostPort());        

        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:somewhere.sipfoundry.org:333", toString(url));

        UtlString hostWithPort;
        url.getHostWithPort(hostWithPort);
        ASSERT_STR_EQUAL("somewhere.sipfoundry.org:333", hostWithPort.data());
    }

    void testBogusPort()
    {
        const char *szUrl = "sip:1234@sipserver:abcd";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:1234@sipserver", toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipserver", getHostAddress(url));
	// The port will be returned as PORT_NONE, because Url::Url()
	// could not parse it.
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, PORT_NONE, url.getHostPort());
    }

    void testIPv6Host()
    {
        const char *szUrl = "[a0:32:44::99]:333";
        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "[a0:32:44::99]", getHostAddress(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 333, url.getHostPort());        

        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sip:[a0:32:44::99]:333", toString(url));
    }


    void testNoBracketUrlWithAllParamsWithVaryingSpace()
    {
        const char *szUrl = "<sip:fsmith@sipfoundry.org:5555 ? call-id=12345 > ; "
            "msgId=5 ;msgId=6;transport=TCP";
        const char *szUrlCorrected = "<sip:fsmith@sipfoundry.org:5555?call-id=12345>;"
            "msgId=5;msgId=6;transport=TCP";

        Url url(szUrl);
        ASSERT_STR_EQUAL_MESSAGE(szUrl, szUrlCorrected, toString(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "sipfoundry.org", getHostAddress(url));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 5555, url.getHostPort());        
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "fsmith", getUserId(url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "12345", getHeaderParam("call-Id", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "5", getFieldParam("msgId", url));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "5", getFieldParam("msgId", url, 0));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "6", getFieldParam("msgId", url, 1));
        ASSERT_STR_EQUAL_MESSAGE(szUrl, "TCP", getFieldParam("transport", url));
    }

    void testConstruction()
    {
        Url url;
        url.setUrlType("sip");
        url.setUserId("someuser") ; 
        url.setHostAddress("main.sip") ; 
        ASSERT_STR_EQUAL("sip:someuser@main.sip", toString(url));
        url.includeAngleBrackets() ; 
        ASSERT_STR_EQUAL("<sip:someuser@main.sip>", toString(url));
    }

    void testHttpConstruction()
    {
        Url url;
        ASSERT_STR_EQUAL("sip:", toString(url));
        url.setUrlType("http") ; 
        url.setHostAddress("web.server") ; 
        url.setPath("/somewhere/in/cyber") ; 
        url.setHostPort(8080) ; 
        ASSERT_STR_EQUAL("http://web.server:8080/somewhere/in/cyber", 
            toString(url));
    }

    void testComplexConstruction()
    {
        Url url;

        // Type should be set to sip by default. Verify that by not setting 
        // anything for the type
        ASSERT_STR_EQUAL("sip:", toString(url));

        url.setUserId("raghu");
        url.setPassword("rgpwd");
        url.setHostAddress("sf.org");
        ASSERT_STR_EQUAL("sip:raghu:rgpwd@sf.org", toString(url));

        url.setUrlParameter("up1", "uval1");
        url.setUrlParameter("up2", "uval2");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;up1=uval1;up2=uval2>", toString(url));

        url.setHeaderParameter("hp1", "hval1");
        url.setHeaderParameter("hp2", "hval2");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>",
                         toString(url));

        url.setFieldParameter("fp1", "fval1");
        ASSERT_STR_EQUAL("<sip:raghu:rgpwd@sf.org;"
                         "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1", toString(url));

        url.setDisplayName("Raghu Venkataramana");
        ASSERT_STR_EQUAL("Raghu Venkataramana<sip:raghu:rgpwd@sf.org;"
                         "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1", toString(url));
    }

    void testAddAttributesToExisting()
    {
        Url url("sip:u@host");

        url.setDisplayName("New Name");
        ASSERT_STR_EQUAL("New Name<sip:u@host>", toString(url));

        url.setHostPort(5070);
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070>", toString(url));

        url.setUrlParameter("u1", "uv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1>", toString(url));

        url.setHeaderParameter("h1", "hv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>", toString(url));

        url.setFieldParameter("f1", "fv1");
        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1", toString(url));

        Url url2("sip:u@host");

        url2.setFieldParameter("field", "value2");
        ASSERT_STR_EQUAL("<sip:u@host>;field=value2", toString(url2));

    }

    void testChangeValues()
    {
        Url url("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1"); 

        ASSERT_STR_EQUAL("New Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));

        url.setDisplayName("Changed Name");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host:5070;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));

        url.setHostPort(PORT_NONE);
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1>;f1=fv1",
                         toString(url));
        
        url.setHeaderParameter("h1", "hv2");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2>;f1=fv1",
                         toString(url));
        
        url.setHeaderParameter("expires", "10");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=10>;f1=fv1",
                         toString(url));
        
        url.setHeaderParameter("expires", "20");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20>;f1=fv1",
                         toString(url));
        
        url.setFieldParameter("f1", "fv2");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20>;f1=fv1;f1=fv2",
                         toString(url));
        
        url.setHeaderParameter("route", "rt1");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20&route=rt1>;f1=fv1;f1=fv2",
                         toString(url));
        
        url.setHeaderParameter("ROUTE", "rt2,rt1");
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1?h1=hv1&h1=hv2&expires=20&ROUTE=rt2%2Crt1>;f1=fv1;f1=fv2",
                         toString(url));
        


        // Only the changed attributes should actually changed
    }

    void testRemoveAttributes()
    {
        Url url("Changed Name<sip:u@host;u1=uv1;u2=uv2"
                "?h1=hv1&h2=hnew2>;f1=fv1;f2=fv2");
        url.removeHeaderParameter("h1") ; 
        url.removeFieldParameter("f1") ; 
        url.removeUrlParameter("u1") ; 

        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u2=uv2?h2=hnew2>;f2=fv2", 
            toString(url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getHeaderParam("h1", url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getFieldParam("f1", url));
        ASSERT_STR_EQUAL(MISSING_PARAM, getParam("u1", url));
    }

    // Test that removeUrlParameter is case-insensitive in parameter names.
    void testRemoveUrlParameterCase()
    {
        Url url1("<sip:600-3@cdhcp139.pingtel.com;q=0.8>");
        url1.removeUrlParameter("q");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com;q=0.8;z=q>");
        url2.removeUrlParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;z=q>",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8>");
        url3.removeUrlParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;abcd=27>",
                         toString(url3));

        Url url4("<sip:600-3@cdhcp139.pingtel.com;Q=0.8;CXV>");
        url4.removeUrlParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com;CXV>",
                         toString(url4));

        Url url5("<sip:600-3@cdhcp139.pingtel.com;mIxEdCaSe=0.8>");
        url5.removeUrlParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    // Test that removeFieldParameter is case-insensitive in parameter names.
    void testRemoveFieldParameterCase()
    {
        Url url1("<sip:600-3@cdhcp139.pingtel.com>;q=0.8");
        url1.removeFieldParameter("q");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com>;q=0.8;z=q");
        url2.removeFieldParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;z=q",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8");
        url3.removeFieldParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;abcd=27",
                         toString(url3));

        Url url4("<sip:600-3@cdhcp139.pingtel.com>;Q=0.8;CXV");
        url4.removeFieldParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com>;CXV",
                         toString(url4));

        Url url5("<sip:600-3@cdhcp139.pingtel.com>;mIxEdCaSe=0.8");
        url5.removeFieldParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    // Test that removeHeaderParameter is case-insensitive in parameter names.
    void testRemoveHeaderParameterCase()
    {
        Url url1("<sip:600-3@cdhcp139.pingtel.com&q=0.8>");
        url1.removeHeaderParameter("q");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url1));

        Url url2("<sip:600-3@cdhcp139.pingtel.com?q=0.8&z=q>");
        url2.removeHeaderParameter("Q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com?z=q>",
                         toString(url2));

        Url url3("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8>");
        url3.removeHeaderParameter("q");

        ASSERT_STR_EQUAL("<sip:600-3@cdhcp139.pingtel.com?abcd=27>",
                         toString(url3));

        Url url5("<sip:600-3@cdhcp139.pingtel.com?mIxEdCaSe=0.8>");
        url5.removeHeaderParameter("MiXeDcAsE");

        ASSERT_STR_EQUAL("sip:600-3@cdhcp139.pingtel.com",
                         toString(url5));
    }

    void testRemoveAllTypesOfAttributes()
    {
        const char* szUrl = "Changed Name<sip:u@host;u1=uv1;u2=uv2"
            "?h1=hv1&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2";

        Url noHeader(szUrl);
        noHeader.removeHeaderParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1;u2=uv2>;f1=fv1;f2=fv2", 
            toString(noHeader));

        Url noUrl(szUrl);
        noUrl.removeUrlParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host?h1=hv1&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2",
            toString(noUrl));

        Url noField(szUrl);
        noField.removeFieldParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host;u1=uv1;u2=uv2?h1=hv1&h2=hchanged2&h3=hnew3>", 
            toString(noField));

        Url noParameters(szUrl);
        noParameters.removeParameters();
        ASSERT_STR_EQUAL("Changed Name<sip:u@host>", 
            toString(noParameters));
    }

    void testRemoveAngleBrackets()
    {
        Url url("<sip:u@host:5070>") ; 
        url.removeAngleBrackets() ; 
        ASSERT_STR_EQUAL("sip:u@host:5070", toString(url));
    }

    void testReset()
    {
        Url url("Changed Name<sip:u@host;u1=uv1;u2=uv2?h1=hv1"
                "&h2=hchanged2&h3=hnew3>;f1=fv1;f2=fv2") ;
        url.reset(); 
        ASSERT_STR_EQUAL("sip:", toString(url));
    }

    void testAssignment()
    {
        const char* szUrl = "Raghu Venkataramana<sip:raghu:rgpwd@sf.org;"
                "up1=uval1;up2=uval2?hp1=hval1&hp2=hval2>;fp1=fval1";
        Url equalsSz = szUrl;
        ASSERT_STR_EQUAL(szUrl, toString(equalsSz));

        Url equalsUrl = Url(szUrl);
        ASSERT_STR_EQUAL(szUrl, toString(equalsUrl));
    }

    void testGetAllParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "<sip:1234@ss.org;u1=uv1;u2=uv2;u3=uv3?h1=hv1&h2=hv2>;"
            "f1=fv1;f2=fv2;f3=fv3";

        // URL params
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "Test false when invalid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getUrlParameters(0, NULL, NULL, paramCount));

        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(szUrl, url.getUrlParameters(3, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(szUrl, 3, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2 u3", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "uv1 uv2 uv3", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "header params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "hv1 hv2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(3, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 3, paramCount);

        sprintf(msg, "field params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2 f3", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "fv1 fv2 fv3", paramValues);
    }

    void testGetDuplicateNamedParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server:5050;p1=u1;p2=u2;p1=u3?"
            "p1=h1&p2=h2>;p1=f1;p2=f2";

        // URL params
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getUrlParameters(3, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 3, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2 p1", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2 u3", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "header params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "field params  %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "p1 p2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2", paramValues);
    }

    void testGetOnlyUrlParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server;up1=u1;up2=u2>";
        // URL params
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getUrlParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "up1 up2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "u1 u2", paramValues);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getHeaderParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getFieldParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);
    }


    void testGetOnlyHeaderParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server?h1=hv1&h2=hv2>";
        // URL params
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getUrlParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getHeaderParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "h1 h2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "hv1 hv2", paramValues);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getFieldParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);
    }

    void testGetOnlyFieldParameters()
    {
        UtlString paramNames[16];
        UtlString paramValues[16];
        int paramCount = 0;

        const char* szUrl = "D Name<sip:abc@server>;f1=fv1;f2=fv2";
        // URL params
        Url url(szUrl);
        char msg[1024];
        sprintf(msg, "Test true when valid arguments %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getUrlParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Header params
        sprintf(msg, "valid header param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.getHeaderParameters(10, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 0, paramCount);

        // Field params
        sprintf(msg, "valid field param count %s", szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.getFieldParameters(2, paramNames, 
            paramValues, paramCount));
        CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, 2, paramCount);

        sprintf(msg, "Test valid arguments %s", szUrl);
        ASSERT_ARRAY_MESSAGE(msg, "f1 f2", paramNames);
        ASSERT_ARRAY_MESSAGE(msg, "fv1 fv2", paramValues);
    }

    // Test that getUrlParameter is case-insensitive in parameter names.
    void testGetUrlParameterCase()
    {
       UtlString value;

       Url url1("<sip:600-3@cdhcp139.pingtel.com;q=0.8>");

       url1.getUrlParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com;q=0.8;z=q>");

       url2.getUrlParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8>");

       url3.getUrlParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url4("<sip:600-3@cdhcp139.pingtel.com;Q=0.8;CXV>");

       url4.getUrlParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com;mIxEdCaSe=0.8>");

       url5.getUrlParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a paramerter that is present multiple times.
       // (Though multiple presences aren't allowed by RFC 3261 sec. 19.1.1.)

       Url url6("<sip:600-3@cdhcp139.pingtel.com;abcd=27;Q=0.8;Abcd=\"12\";ABCD=EfG>");

       url6.getUrlParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("abcd", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getUrlParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("ABCD", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getUrlParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getUrlParameter("aBCd", value, 1);
       // Double-quotes do not quote URI parameters.
       ASSERT_STR_EQUAL("\"12\"", value.data());
       url6.getUrlParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    // Test that getFieldParameter is case-insensitive in parameter names.
    void testGetFieldParameterCase()
    {
       UtlString value;

       Url url00("<sip:333@212.247.206.174:2052;transport=tcp;line=98tq8dsn>;q=1.0;+sip.instance=\"<urn:uuid:1d960183-88c9-4813-80e1-b97946c09465>\";audio;mobility=\"fixed\";duplex=\"full\";description=\"snom320\";actor=\"principal\";events=\"dialog\";methods=\"INVITE,ACK,CANCEL,BYE,REFER,OPTIONS,NOTIFY,SUBSCRIBE,PRACK,MESSAGE,INFO\"");

       url00.getFieldParameter("+sip.instance", value, 0);

       ASSERT_STR_EQUAL("<urn:uuid:1d960183-88c9-4813-80e1-b97946c09465>",
                        value.data());

       Url url1("<sip:600-3@cdhcp139.pingtel.com>;q=0.8");

       url1.getFieldParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com>;q=0.8;z=q");

       url2.getFieldParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8");

       url3.getFieldParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url4("<sip:600-3@cdhcp139.pingtel.com>;Q=0.8;CXV");

       url4.getFieldParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com>;mIxEdCaSe=0.8");

       url5.getFieldParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a paramerter that is present multiple times.
       // (Though multiple presences aren't allowed by RFC 3261 sec. 19.1.1.)

       Url url6("<sip:600-3@cdhcp139.pingtel.com>;abcd=27;Q=0.8;Abcd=\"12\";ABCD=EfG");

       url6.getFieldParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("abcd", value, 1);
       // getFieldParameter now de-quotes field values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getFieldParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("ABCD", value, 1);
       // getFieldParameter now un-quotes quoted values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getFieldParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getFieldParameter("aBCd", value, 1);
       // getFieldParameter now de-quotes field values.
       ASSERT_STR_EQUAL("12", value.data());
       url6.getFieldParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    // Test that getHeaderParameter is case-insensitive in parameter names.
    void testGetHeaderParameterCase()
    {
       UtlString value;

       Url url1("<sip:600-3@cdhcp139.pingtel.com?q=0.8>");

       url1.getHeaderParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url2("<sip:600-3@cdhcp139.pingtel.com?q=0.8&z=q>");

       url2.getHeaderParameter("Q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url3("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8>");

       url3.getHeaderParameter("q", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       Url url5("<sip:600-3@cdhcp139.pingtel.com?mIxEdCaSe=0.8>");

       url5.getHeaderParameter("MiXeDcAsE", value, 0);
       ASSERT_STR_EQUAL("0.8", value.data());

       // Fetch instances of a header that is present multiple times.

       Url url6("<sip:600-3@cdhcp139.pingtel.com?abcd=27&Q=0.8&Abcd=12&ABCD=EfG>");

       url6.getHeaderParameter("abcd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("abcd", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("abcd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getHeaderParameter("ABCD", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("ABCD", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("ABCD", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());

       url6.getHeaderParameter("aBCd", value, 0);
       ASSERT_STR_EQUAL("27", value.data());
       url6.getHeaderParameter("aBCd", value, 1);
       ASSERT_STR_EQUAL("12", value.data());
       url6.getHeaderParameter("aBCd", value, 2);
       ASSERT_STR_EQUAL("EfG", value.data());
    }

    void testIsDigitString()
    {
        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a single digit", 
            Url::isDigitString("1")) ; 

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a long numeric string", 
            Url::isDigitString("1234567890234586")) ; 

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString returns false for an alpha string", 
            !Url::isDigitString("abcd")) ; 

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString returns false for alpha then pound key", 
            !Url::isDigitString("1234#")) ; 

        CPPUNIT_ASSERT_MESSAGE("Verify isDigitString for a string that has a star key in it", 
            Url::isDigitString("*6")) ; 
    }

    void testIsUserHostPortEqualExact()
    {        
        const char *szUrl = "Raghu Venkatarmana<sip:raghu@sf.org:5080;blah=b?bl=a>;bdfdf=ere";
        const char *szTest                       = "raghu@sf.org:5080";
        Url url(szUrl);
        
        char msg[1024];
        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortVaryCapitalization()
    {
        const char *szUrl = "R V<sip:raghu@SF.oRg:5080>";
        const char *szTest =        "raghu@sf.org:5080";
        Url url(szUrl);
        
        char msg[1024];
        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortNoPort()
    {
        const char *szUrl = "R V<sip:raghu@sf.org>";
        const char *szTest =        "raghu@sf.org:5060";
        Url url(szUrl);
        
        char msg[1024];
        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.isUserHostPortEqual(szTest));
        CPPUNIT_ASSERT_MESSAGE(msg, url.isUserHostPortEqual(szTest, 5060));
    }

    void testIsUserHostPortNoMatch()
    {
        const char *szUrl = "R V<sip:Raghu@SF.oRg:5080>";
        const char *szTest =        "raghu@sf.org:5080";
        Url url(szUrl);
        
        char msg[1024];
        sprintf(msg, "test=%s, url=%s", szTest, szUrl);
        CPPUNIT_ASSERT_MESSAGE(msg, !url.isUserHostPortEqual(szTest));
    }

    void testIsUserHostPortPorts()
    {
       // Urls which are all the same.
       {
          const char* strings[] = {
             "R V<sip:Raghu@SF.oRg:5080>",
             "<sip:Raghu@SF.oRg:5080>",
             "Raghu@SF.oRg:5080",
             // Make sure that case differences are handled correctly.
             "Raghu@sf.org:5080",
          };
          Url urls[sizeof (strings) / sizeof (strings[0])];
          unsigned int i;

          // Set up the Url objects.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             urls[i] = strings[i];
          }

          // Do all the comparisons.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             for (unsigned int j = 0;
                  j < sizeof (strings) / sizeof (strings[0]);
                  j++)
             {
                int expected = TRUE;
                int actual = urls[i].isUserHostPortEqual(urls[j]);
                char msg[80];
                sprintf(msg, "%s != %s", strings[i], strings[j]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expected, actual);
             }
          }
       }

       // Urls which are all different.
       {
          const char* strings[] = {
             "sip:foo@bar",
             "sip:foo@bar:5060",
             "sip:foo@bar:1",
             "sip:foo@bar:100",
             "sip:foo@bar:65535",
             // Make sure case differences are detected.
             "sip:Foo@bar",
          };
          Url urls[sizeof (strings) / sizeof (strings[0])];
          unsigned int i;

          // Set up the Url objects.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             urls[i] = strings[i];
          }

          // Do all the comparisons.
          for (i = 0; i < sizeof (strings) / sizeof (strings[0]);
               i++)
          {
             for (unsigned int j = 0;
                  j < sizeof (strings) / sizeof (strings[0]);
                  j++)
             {
                int expected = (i == j);
                int actual = urls[i].isUserHostPortEqual(urls[j]);
                char msg[80];
                sprintf(msg, "%s == %s", strings[i], strings[j]);
                CPPUNIT_ASSERT_EQUAL_MESSAGE(msg, expected, actual);
             }
          }
       }
    }

    void testToString()
    {
        const char* szUrl = "sip:192.168.1.102" ;
        Url url(szUrl) ;
        UtlString toString("SHOULD_BE_REPLACED");
        url.toString(toString) ;

        // Verify that toString replaces (as opposed to append)
        ASSERT_STR_EQUAL(szUrl, toString.data()) ;
    }

    void testFromString()
    {
        Url url;

        UtlString inputUrl("sip:192.168.1.102");
        url.fromString(inputUrl);

        UtlString firstString("SHOULD_BE_REPLACED");
        url.toString(firstString) ;

        ASSERT_STR_EQUAL(inputUrl.data(), firstString.data()) ;

        UtlString rewrittenUrl("sip:user@host");
        url.fromString(rewrittenUrl);

        UtlString secondString("SHOULD_BE_REPLACED");
        url.toString(secondString) ;

        ASSERT_STR_EQUAL(rewrittenUrl.data(), secondString.data()) ;
    }


    void testGetIdentity()
    {
       // The data for a single test.
       struct test {
          // The string to convert to a URI.
          const char* string;
          // The identity to be returned by getIdentity().
          const char* identity;
       };
       // The tests.
       struct test tests[] = {
          { "sip:foo@bar", "foo@bar" },
          { "sip:foo@bar:5060", "foo@bar:5060" },
          { "sip:foo@bar:1", "foo@bar:1" },
          { "sip:foo@bar:100", "foo@bar:100" },
          { "sip:foo@bar:65535", "foo@bar:65535" },
          { "<sip:foo@bar>", "foo@bar" },
          { "<sip:foo@bar:5060>", "foo@bar:5060" },
          { "<sip:foo@bar:1>", "foo@bar:1" },
          { "<sip:foo@bar:100>", "foo@bar:100" },
          { "<sip:foo@bar:65535>", "foo@bar:65535" },
       };

       for (unsigned int i = 0; i < sizeof (tests) / sizeof (tests[0]);
            i++)
       {
          Url url = tests[i].string;
          ASSERT_STR_EQUAL_MESSAGE(tests[i].string, tests[i].identity,
                                   getIdentity(url));
       }
    }

/*
 * The primary purpose of the following tests is to look for cases in which
 * the regular expressions either take too long to run, or recurse too deeply
 * (the latter will usually also cause the former).
 *
 * Because we don't want the tests to be chatty, and we don't want to pick an
 * arbitrary time for "too long", the following PARSE macro.  It wraps the
 * constructor and in the verbose form prints how long it took to run.
 * Normally, this should be disabled, but turn it on if you're making changes
 * to the regular expressions so that you can see any performance/recursion
 * problems.
 */
#if 0
#     define PARSE(name)                        \
      OsTimeLog name##timeLog;                  \
      name##timeLog.addEvent("start  " #name);  \
      Url name##Url(name);                      \
      name##timeLog.addEvent("parsed " #name);  \
      UtlString name##Log;                      \
      name##timeLog.getLogString(name##Log);    \
      printf("\n%s\n", name##Log.data());
#else
#     define PARSE(name)                     \
      Url name##Url(name);
#endif
         
    void testNormalUri()
    {
        // exists just to provide a time comparison for the following testBigUri... tests
        UtlString normal("Display Name <sip:user@example.com>");
        PARSE(normal);
    }

    void testBigUriDisplayName()
    {
        // <bigtoken> sip:user@example.com

        UtlString bigname;
        bigname.append(bigtoken);
        bigname.append(" <sip:user@example.com>");

        PARSE(bigname);

        CPPUNIT_ASSERT(bignameUrl.getScheme() == Url::SipUrlScheme);

        UtlString component;

        bignameUrl.getDisplayName(component);
        CPPUNIT_ASSERT(! component.compareTo(bigtoken)); 

        bignameUrl.getUserId(component);
        CPPUNIT_ASSERT(! component.compareTo("user")); 

        bignameUrl.getHostAddress(component);
        CPPUNIT_ASSERT(! component.compareTo("example.com"));          
    }

    void testBigUriQuotedName()
    {
        // "<bigtoken>" sip:user@example.com

        UtlString bigquotname;
        bigquotname.append("\"");
        bigquotname.append(bigtoken);
        bigquotname.append("\" <sip:user@example.com>");

        PARSE(bigquotname);

        UtlString component;

        bigquotnameUrl.getDisplayName(component);
        UtlString quoted_bigtoken;
        quoted_bigtoken.append("\"");
        quoted_bigtoken.append(bigtoken);
        quoted_bigtoken.append("\"");
        CPPUNIT_ASSERT(! component.compareTo(quoted_bigtoken)); 

        CPPUNIT_ASSERT(bigquotnameUrl.getScheme() == Url::SipUrlScheme);

        bigquotnameUrl.getUserId(component);
        CPPUNIT_ASSERT(! component.compareTo("user")); 

        bigquotnameUrl.getHostAddress(component);
        CPPUNIT_ASSERT(! component.compareTo("example.com"));
    }

    void testBigUriScheme()
      {
         // <bigtoken>:user:password@example.com
         
         UtlString bigscheme;
         bigscheme.append(bigtoken);
         bigscheme.append(":user:password@example.com");
         
         PARSE(bigscheme);

         UtlString component;

         CPPUNIT_ASSERT(bigschemeUrl.getScheme() == Url::UnknownUrlScheme); 

         bigschemeUrl.getUserId(component);
         CPPUNIT_ASSERT(component.isNull()); 
         
         bigschemeUrl.getHostAddress(component);
         CPPUNIT_ASSERT(component.isNull());
         
         Url bigSchemeAddrType(bigscheme, TRUE /* as addr-type */);

         CPPUNIT_ASSERT(bigSchemeAddrType.getScheme() == Url::UnknownUrlScheme); // ?

         bigSchemeAddrType.getUserId(component);
         CPPUNIT_ASSERT(component.isNull()); // bigtoken
         
         bigSchemeAddrType.getHostAddress(component);
         CPPUNIT_ASSERT(component.isNull());        
      }

   void testBigUriUser()
      {
         // sip:<bigtoken>@example.com
         
         UtlString biguser;
         biguser.append("sip:");
         biguser.append(bigtoken);
         biguser.append("@example.com");
         
         PARSE(biguser);

         CPPUNIT_ASSERT(biguserUrl.getScheme() == Url::SipUrlScheme);
         
         UtlString component;

         biguserUrl.getUserId(component);
         CPPUNIT_ASSERT(component.compareTo(bigtoken) == 0);
         
         biguserUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriNoSchemeUser()
      {
         // <bigtoken>@example.com
         
         UtlString bigusernoscheme;

         bigusernoscheme.append(bigtoken);
         bigusernoscheme.append("@example.com");
         
         PARSE(bigusernoscheme);

         CPPUNIT_ASSERT(bigusernoschemeUrl.getScheme() == Url::SipUrlScheme);
         
         UtlString component;

         bigusernoschemeUrl.getUserId(component);
         CPPUNIT_ASSERT(component.compareTo(bigtoken) == 0);
         
         bigusernoschemeUrl.getHostAddress(component);
         CPPUNIT_ASSERT(! component.compareTo("example.com"));
      }

   void testBigUriHost()
      {
         
         // see if a 128 character host parses ok
         UtlString okhost("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa");
         UtlString bigok;
         bigok.append("sip:user@");
         bigok.append(okhost);
         
         PARSE(bigok);

         CPPUNIT_ASSERT(bigokUrl.getScheme() == Url::SipUrlScheme);

         UtlString component;

         bigokUrl.getUserId(component);
         CPPUNIT_ASSERT(!component.compareTo("user"));
         
         bigokUrl.getHostAddress(component);
         CPPUNIT_ASSERT(!component.compareTo(okhost));

         // user@<bigtoken>
         /*
          * A really big host name like this causes recursion in the regular expression
          * that matches a domain name; the limit causes this match to fail.
          * This is preferable to having the match succeed but either take minutes (yes,
          * minutes) to do the match and/or overflow the stack, so we let it fail.
          */

         UtlString bighost;

         bighost.append("sip:user@");
         bighost.append(bigtoken);
         
         PARSE(bighost);

         CPPUNIT_ASSERT(bighostUrl.getScheme() == Url::UnknownUrlScheme);
      }

    /////////////////////////
    // Helper Methods

    const char *getParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getUrlParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getHeaderParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getHeaderParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url, int ndx)
    {
        UtlString name(szName);        
        if (!url.getFieldParameter(name, *assertValue, ndx))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *getFieldParam(const char *szName, Url &url)
    {
        UtlString name(szName);        
        if (!url.getFieldParameter(name, *assertValue))
        {
            assertValue->append(MISSING_PARAM);
        }

        return assertValue->data();
    }

    const char *toString(const Url& url)
    {
        assertValue->remove(0);
        url.toString(*assertValue);

        return assertValue->data();
    }

    const char *getHostAddress(const Url& url)
    {
        assertValue->remove(0);
        url.getHostAddress(*assertValue);

        return assertValue->data();
    }

    const char *getUrlType(const Url& url)
    {
        assertValue->remove(0);
        url.getUrlType(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getUri(Url& url)
    {
        assertValue->remove(0);
        url.getUri(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getPath(Url& url, UtlBoolean withQuery = FALSE)
    {
        assertValue->remove(0);
        url.getPath(*assertValue, withQuery);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getUserId(Url& url)
    {
        assertValue->remove(0);
        url.getUserId(*assertValue);

        return assertValue->data();
    }

    /** API not declared as const **/
    const char *getIdentity(Url& url)
    {
        assertValue->remove(0);
        url.getIdentity(*assertValue);

        return assertValue->data();
    }

    const char *getDisplayName(const Url &url)
    {
        assertValue->remove(0);
        url.getDisplayName(*assertValue);

        return assertValue->data();
    }

    void assertArrayMessage(const char *expectedTokens, UtlString *actual, 
        CppUnit::SourceLine sourceLine, std::string msg)
    {
        UtlString expected;
        UtlTokenizer toks(expectedTokens);
        for (int i = 0; toks.next(expected, " "); i++)
        {
            TestUtilities::assertEquals(expected.data(), actual[i].data(), 
                sourceLine, msg);
            expected.remove(0);
        }
    }
}; 

const size_t UrlTest::BIG_SIZE=8200;

CPPUNIT_TEST_SUITE_REGISTRATION(UrlTest);
