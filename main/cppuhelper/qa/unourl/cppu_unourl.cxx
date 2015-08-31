/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#include "cppuhelper/unourl.hxx"
#include "rtl/malformeduriexception.hxx"
#include "rtl/strbuf.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustring.hxx"
#include "sal/types.h"
#include "gtest/gtest.h"

namespace cppu_unourl
{
    class UrlTest : public ::testing::Test
    {
    public:
    };

    TEST_F(UrlTest, testDescriptorParsing)
    {
        struct Test
        {
            char const * pInput;
            bool bValid;
        };
        static Test const aTests[]
            = { { "", false },
                { "abc", true },
                { "Abc", true },
                { "aBC", true },
                { "ABC", true },
                { "1abc", true },
                { "123", true },
                { "abc-1", false },
                { "ab%63", false },
                { "abc,", false },
                { "abc,def=", true },
                { "abc,Def=", true },
                { "abc,DEF=", true },
                { "abc,1def=", true },
                { "abc,123=", true },
                { "abc,def-1=", false },
                { "abc,def", false },
                { "abc,def=xxx,def=xxx", false },
                { "abc,def=xxx,ghi=xxx", true },
                { "abc,,def=xxx", false },
                { "abc,def=xxx,,ghi=xxx", false },
                { "abc,def=xxx,ghi=xxx,", false },
                { "abc,def=%", true },
                { "abc,def=%1", true },
                { "abc,def=%00", true },
                { "abc,def=%22", true },
                { "abc,def=\"", true },
                { "abc,def=%ed%a0%80", true } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            try
            {
                cppu::UnoUrlDescriptor aDescriptor(rtl::OUString::createFromAscii(
                                                       aTests[i].pInput));
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}

            if (aTests[i].bValid)
            {
                ASSERT_TRUE(bValid) << "Valid uri parsed as invalid";
            }
            else
            {
                ASSERT_TRUE(!bValid) << "Invalid uri parsed as valid";
            }
        }
    }

    TEST_F(UrlTest, testDescriptorDescriptor)
    {
        struct Test
        {
            char const * pInput;
            char const * pDescriptor;
        };
        static Test const aTests[]
            = {{ "abc", "abc" },
               { "Abc", "Abc" },
               { "aBC", "aBC" },
               { "ABC", "ABC" },
               { "1abc", "1abc" },
               { "123", "123" },
               { "abc,def=", "abc,def=" },
               { "abc,Def=", "abc,Def=" },
               { "abc,DEF=", "abc,DEF=" },
               { "abc,1def=", "abc,1def=" },
               { "abc,123=", "abc,123=" },
               { "abc,def=xxx,ghi=xxx", "abc,def=xxx,ghi=xxx" },
               { "abc,def=%", "abc,def=%" },
               { "abc,def=%1", "abc,def=%1" },
               { "abc,def=%00", "abc,def=%00" },
               { "abc,def=%22", "abc,def=%22" },
               { "abc,def=\"", "abc,def=\"" },
               { "abc,def=%ed%a0%80", "abc,def=%ed%a0%80" } };
        bool bResult = true;
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aDescriptor;
            try
            {
                aDescriptor = cppu::UnoUrlDescriptor(rtl::OUString::createFromAscii(
                                                         aTests[i].pInput)).
                   getDescriptor();
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}

            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aDescriptor.equalsAscii(aTests[i].pDescriptor)) << "Failed to parse URI correctly";
        }
    }


    TEST_F(UrlTest, testDescriptorName)
    {
        struct Test
        {
            char const * pInput;
            char const * pName;
        };
        static Test const aTests[]
            = { { "abc", "abc" },
                { "Abc", "abc" },
                { "aBC", "abc" },
                { "ABC", "abc" },
                { "1abc", "1abc" },
                { "123", "123" },
                { "abc,def=", "abc" },
                { "abc,Def=", "abc" },
                { "abc,DEF=", "abc" },
                { "abc,1def=", "abc" },
                { "abc,123=", "abc" },
                { "abc,def=xxx,ghi=xxx", "abc" },
                { "abc,def=%", "abc" },
                { "abc,def=%1", "abc" },
                { "abc,def=%00", "abc" },
                { "abc,def=%22", "abc" },
                { "abc,def=\"", "abc" },
                { "abc,def=%ed%a0%80", "abc" } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aName;
            try
            {
                aName = cppu::UnoUrlDescriptor(rtl::OUString::createFromAscii(
                                                   aTests[i].pInput)).getName();
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}

            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aName.equalsAscii(aTests[i].pName)) << "Failed to parse URI correctly";
        }
    }

    TEST_F(UrlTest, testDescriptorKey)
    {
        struct Test
        {
            char const * pInput;
            char const * pKey;
            bool bPresent;
        };
        static Test const aTests[]
            = { { "abc", "abc", false },
                { "abc", "def", false },
                { "1abc", "def", false },
                { "123", "def", false },
                { "abc,def=", "abc", false },
                { "abc,def=", "def", true },
                { "abc,def=", "defg", false },
                { "abc,def=", "de", false },
                { "abc,def=", "ghi", false },
                { "abc,Def=", "def", true },
                { "abc,Def=", "Def", true },
                { "abc,Def=", "dEF", true },
                { "abc,Def=", "DEF", true },
                { "abc,def=xxx,ghi=xxx", "abc", false },
                { "abc,def=xxx,ghi=xxx", "def", true },
                { "abc,def=xxx,ghi=xxx", "ghi", true },
                { "abc,def=xxx,ghi=xxx", "jkl", false } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            bool bPresent = false;
            try
            {
                bPresent = cppu::UnoUrlDescriptor(rtl::OUString::createFromAscii(
                                                      aTests[i].pInput)).
                    hasParameter(rtl::OUString::createFromAscii(aTests[i].pKey));
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}

            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(bPresent == aTests[i].bPresent) << "Failed to detect parameter correctly";
        }
    }

    TEST_F(UrlTest, testDescriptorValue)
    {
        struct Test
        {
            char const * pInput;
            char const * pKey;
            char const * pValue;
        };
        static Test const aTests[]
            = { { "abc", "abc", "" },
                { "abc", "def", "" },
                { "1abc", "def", "" },
                { "123", "def", "" },
                { "abc,def=", "abc", "" },
                { "abc,def=", "def", "" },
                { "abc,def=", "defg", "" },
                { "abc,def=", "de", "" },
                { "abc,def=", "ghi", "" },
                { "abc,Def=", "def", "" },
                { "abc,Def=", "Def", "" },
                { "abc,Def=", "dEF", "" },
                { "abc,Def=", "DEF", "" },
                { "abc,def=xxx,ghi=xxx", "abc", "" },
                { "abc,def=xxx,ghi=xxx", "def", "xxx" },
                { "abc,def=xxx,ghi=xxx", "ghi", "xxx" },
                { "abc,def=xxx,ghi=xxx", "jkl", "" },
                { "abc,def=%", "def", "%" },
                { "abc,def=%1", "def", "%1" },
                { "abc,def=%22", "def", "\"" },
                { "abc,def=\"", "def", "\"" },
                { "abc,def=abc", "def", "abc" },
                { "abc,def=Abc", "def", "Abc" },
                { "abc,def=aBC", "def", "aBC" },
                { "abc,def=ABC", "def", "ABC" },
                { "abc,def=%,ghi=", "def", "%" },
                { "abc,def=%1,ghi=", "def", "%1" },
                { "abc,def=%22,ghi=", "def", "\"" },
                { "abc,def=\",ghi=", "def", "\"" },
                { "abc,def=abc,ghi=", "def", "abc" },
                { "abc,def=Abc,ghi=", "def", "Abc" },
                { "abc,def=aBC,ghi=", "def", "aBC" },
                { "abc,def=ABC,ghi=", "def", "ABC" },
                { "abc,abc=,def=%", "def", "%" },
                { "abc,abc=,def=%1", "def", "%1" },
                { "abc,abc=,def=%22", "def", "\"" },
                { "abc,abc=,def=\"", "def", "\"" },
                { "abc,abc=,def=abc", "def", "abc" },
                { "abc,abc=,def=Abc", "def", "Abc" },
                { "abc,abc=,def=aBC", "def", "aBC" },
                { "abc,abc=,def=ABC", "def", "ABC" } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aValue;
            try
            {
                aValue = cppu::UnoUrlDescriptor(rtl::OUString::createFromAscii(
                                                    aTests[i].pInput)).
                    getParameter(rtl::OUString::createFromAscii(aTests[i].pKey));
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}
            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aValue.equalsAscii(aTests[i].pValue)) << "Failed to get param correctly";
        }
    }

    TEST_F(UrlTest, testUrlParsing)
    {
        struct Test
        {
            char const * pInput;
            bool bValid;
        };
        static Test const aTests[]
            = { { "", false },
                { "abc", false },
                { "uno", false },
                { "uno:", false },
                { "uno:abc;def;ghi", true },
                { "Uno:abc;def;ghi", true },
                { "uNO:abc;def;ghi", true },
                { "UNO:abc;def;ghi", true },
                { "uno:abc,def=xxx,ghi=xxx;def,ghi=xxx,jkl=xxx;ghi", true },
                { "uno:abc,def=xxx,ghi=xxx;def,ghi=xxx,jkl=xxx,;ghi", false },
                { "uno:abc;def;", false },
                { "uno:abc;def;a", true },
                { "uno:abc;def;A", true },
                { "uno:abc;def;1", true },
                { "uno:abc;def;$&+,/:=?@", true },
                { "uno:abc;def;%24&+,/:=?@", false } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            try
            {
                cppu::UnoUrl aUrl(rtl::OUString::createFromAscii(aTests[i].pInput));
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}

            if (aTests[i].bValid)
            {
                ASSERT_TRUE(bValid) << "Valid uri parsed as invalid";
            }
            else
            {
                ASSERT_TRUE(!bValid) << "Invalid uri parsed as valid";
            }

        }
    }

    TEST_F(UrlTest, testUrlConnection)
    {
        struct Test
        {
            char const * pInput;
            char const * pConnection;
        };
        static Test const aTests[]
            = { { "uno:abc;def;ghi", "abc" },
                { "uno:Abc;def;ghi", "Abc" },
                { "uno:aBC;def;ghi", "aBC" },
                { "uno:ABC;def;ghi", "ABC" },
                { "uno:abc,def=xxx,ghi=xxx;def,ghi=xxx,jkl=xxx;ghi",
                  "abc,def=xxx,ghi=xxx" } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aConnection;
            try
            {
                aConnection = cppu::UnoUrl(rtl::OUString::createFromAscii(
                                               aTests[i].pInput)).
                    getConnection().getDescriptor();
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}
            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aConnection.equalsAscii(aTests[i].pConnection)) << "Failed to get param correctly";
        }
    }

    TEST_F(UrlTest, testUrlProtocol)
    {
        struct Test
        {
            char const * pInput;
            char const * pProtocol;
        };
        static Test const aTests[]
            = { { "uno:abc;def;ghi", "def" },
                { "uno:abc;Def;ghi", "Def" },
                { "uno:abc;dEF;ghi", "dEF" },
                { "uno:abc;DEF;ghi", "DEF" },
                { "uno:abc,def=xxx,ghi=xxx;def,ghi=xxx,jkl=xxx;ghi",
                  "def,ghi=xxx,jkl=xxx" } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aProtocol;
            try
            {
                aProtocol = cppu::UnoUrl(rtl::OUString::createFromAscii(
                                             aTests[i].pInput)).
                    getProtocol().getDescriptor();
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}
            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aProtocol.equalsAscii(aTests[i].pProtocol)) << "Failed to get protocol correctly";
        }
    }

    TEST_F(UrlTest, testUrlObjectName)
    {
        struct Test
        {
            char const * pInput;
            char const * pObjectName;
        };
        static Test const aTests[]
            = { { "uno:abc;def;ghi", "ghi" },
                { "uno:abc;def;Ghi", "Ghi" },
                { "uno:abc;def;gHI", "gHI" },
                { "uno:abc;def;GHI", "GHI" },
                { "uno:abc,def=xxx,ghi=xxx;def,ghi=xxx,jkl=xxx;ghi", "ghi" },
                { "uno:abc;def;a", "a" },
                { "uno:abc;def;A", "A" },
                { "uno:abc;def;1", "1" },
                { "uno:abc;def;$&+,/:=?@", "$&+,/:=?@" } };
        for (int i = 0; i < sizeof aTests / sizeof (Test); ++i)
        {
            bool bValid = false;
            rtl::OUString aObjectName;
            try
            {
                aObjectName = cppu::UnoUrl(rtl::OUString::createFromAscii(
                                               aTests[i].pInput)).getObjectName();
                bValid = true;
            }
            catch (rtl::MalformedUriException &)
            {}
            ASSERT_TRUE(bValid) << "Failed to parse URI";
            ASSERT_TRUE(aObjectName.equalsAscii(aTests[i].pObjectName)) << "Failed to get protocol correctly";
        }
    }
} // namespace cppu_ifcontainer

int main(int argc, char **argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

