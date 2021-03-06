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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"


#define _SVSTDARR_STRINGS

#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <vos/mutex.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <svl/svstdarr.hxx>
#include <svtools/unoevent.hxx>
#include <svl/urihelper.hxx>
#include <sfx2/event.hxx>
#include <swtypes.hxx>
#include <glosdoc.hxx>
#include <shellio.hxx>
#include <initui.hxx>
#include <gloslst.hxx>
#include <unoatxt.hxx>
#include <unomap.hxx>
#include <unomid.h>
#include <unotextbodyhf.hxx>
#include <unotextrange.hxx>
#include <TextCursorHelper.hxx>
#include <swevent.hxx>
#include <doc.hxx>
#include <unocrsr.hxx>
#include <IMark.hxx>
#include <unoprnms.hxx>
#include <docsh.hxx>
#include <swunodef.hxx>
#include <swmodule.hxx>
#include <svl/smplhint.hxx>
#include <svl/macitem.hxx>

#include <editeng/acorrcfg.hxx>

#include <memory>


SV_IMPL_REF ( SwDocShell )
using namespace ::com::sun::star;
using ::rtl::OUString;

/******************************************************************
 *
 ******************************************************************/
/* -----------------30.03.99 14:31-------------------
 *
 * --------------------------------------------------*/
uno::Reference< uno::XInterface > SAL_CALL SwXAutoTextContainer_createInstance(
    const uno::Reference< lang::XMultiServiceFactory > & ) throw( uno::Exception )
{
	//the module may not be loaded
	::vos::OGuard aGuard(Application::GetSolarMutex());
	SwDLL::Init();
	static uno::Reference< uno::XInterface > xAText = (cppu::OWeakObject*)new SwXAutoTextContainer();;
	return xAText;
}
/* -----------------------------17.04.01 13:17--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SAL_CALL SwXAutoTextContainer_getSupportedServiceNames() throw()
{
    OUString sService( RTL_CONSTASCII_USTRINGPARAM("com.sun.star.text.AutoTextContainer"));
    const uno::Sequence< OUString > aSeq( &sService, 1 );
	return aSeq;
}
/* -----------------------------17.04.01 13:18--------------------------------

 ---------------------------------------------------------------------------*/
OUString SAL_CALL SwXAutoTextContainer_getImplementationName() throw()
{
    return OUString( RTL_CONSTASCII_USTRINGPARAM("SwXAutoTextContainer" ) );
}

/*-- 21.12.98 12:42:16---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextContainer::SwXAutoTextContainer()
{
	pGlossaries = ::GetGlossaries();

}
/*-- 21.12.98 12:42:17---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextContainer::~SwXAutoTextContainer()
{

}
/*-- 21.12.98 12:42:17---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXAutoTextContainer::getCount(void) throw( uno::RuntimeException )
{
	return pGlossaries->GetGroupCnt();
}
/*-- 21.12.98 12:42:18---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXAutoTextContainer::getByIndex(sal_Int32 nIndex)
	throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	uno::Any aRet;
	sal_uInt16 nCount = pGlossaries->GetGroupCnt();
	if ( 0 <= nIndex && nIndex < nCount )
        aRet = getByName(pGlossaries->GetGroupName( static_cast< sal_uInt16 >(nIndex) ));
	else
		throw lang::IndexOutOfBoundsException();
	return aRet;
}
/*-- 21.12.98 12:42:18---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXAutoTextContainer::getElementType(void) throw( uno::RuntimeException )
{
	return ::getCppuType((const uno::Reference<text::XAutoTextGroup>*)0);

}
/*-- 21.12.98 12:42:18---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXAutoTextContainer::hasElements(void) throw( uno::RuntimeException )
{
	//zumindest Standard sollte es immer geben!
	return sal_True;
}
/*-- 21.12.98 12:42:18---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXAutoTextContainer::getByName(const OUString& GroupName)
	throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());

	uno::Reference< text::XAutoTextGroup > xGroup;
	if ( pGlossaries && hasByName( GroupName ) )	// group name already known?
		// sal_True = create group if not already available
        xGroup = pGlossaries->GetAutoTextGroup( GroupName, sal_True );

	if ( !xGroup.is() )
		throw container::NoSuchElementException();

	return makeAny( xGroup );
}
/*-- 21.12.98 12:42:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextContainer::getElementNames(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	sal_uInt16 nCount = pGlossaries->GetGroupCnt();

	uno::Sequence< OUString > aGroupNames(nCount);
	OUString *pArr = aGroupNames.getArray();

	for ( sal_uInt16 i = 0; i < nCount; i++ )
	{
		// Die Namen werden ohne Pfad-Extension weitergegeben
		String sGroupName(pGlossaries->GetGroupName(i));
		pArr[i] = sGroupName.GetToken(0, GLOS_DELIM);
	}
	return aGroupNames;
}
/*-- 21.12.98 12:42:19---------------------------------------------------
	findet Gruppennamen mit und ohne Pfadindex
  -----------------------------------------------------------------------*/
sal_Bool SwXAutoTextContainer::hasByName(const OUString& Name)
	throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	String sGroupName( pGlossaries->GetCompleteGroupName( Name ) );
	if(sGroupName.Len())
		return sal_True;
	return sal_False;
}
/*-- 21.12.98 12:42:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XAutoTextGroup >  SwXAutoTextContainer::insertNewByName(
	const OUString& aGroupName)
	throw( lang::IllegalArgumentException, container::ElementExistException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	if(hasByName(aGroupName))
		throw container::ElementExistException();
	//check for non-ASCII characters
	if(!aGroupName.getLength())
	{
		lang::IllegalArgumentException aIllegal;
		aIllegal.Message = C2U("group name must not be empty");
		throw aIllegal;
	}
	for(sal_Int32 nPos = 0; nPos < aGroupName.getLength(); nPos++)
	{
		sal_Unicode cChar = aGroupName[nPos];
		if(	((cChar >= 'A') && (cChar <= 'Z')) ||
            ((cChar >= 'a') && (cChar <= 'z')) ||
            ((cChar >= '0') && (cChar <= '9')) ||
            (cChar == '_') ||
            (cChar == 0x20) ||
            (cChar == GLOS_DELIM) )
        {
			continue;
		}
		lang::IllegalArgumentException aIllegal;
        aIllegal.Message = C2U("group name must contain a-z, A-z, '_', ' ' only");
		throw aIllegal;
	}
	String sGroup(aGroupName);
	if(STRING_NOTFOUND == sGroup.Search(GLOS_DELIM))
	{
		sGroup += GLOS_DELIM;
		sGroup += UniString::CreateFromInt32(0);
	}
	pGlossaries->NewGroupDoc(sGroup, sGroup.GetToken(0, GLOS_DELIM));

	uno::Reference< text::XAutoTextGroup > xGroup = pGlossaries->GetAutoTextGroup( sGroup, true );
	DBG_ASSERT( xGroup.is(), "SwXAutoTextContainer::insertNewByName: no UNO object created? How this?" );
		// we just inserted the group into the glossaries, so why doesn't it exist?

	return xGroup;
}
/*-- 21.12.98 12:42:19---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextContainer::removeByName(const OUString& aGroupName)
	throw( container::NoSuchElementException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	//zunaechst den Namen mit Pfad-Extension finden
	String sGroupName = pGlossaries->GetCompleteGroupName( aGroupName );
	if(!sGroupName.Len())
		throw container::NoSuchElementException();
	pGlossaries->DelGroupDoc(sGroupName);
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXAutoTextContainer::getImplementationName(void) throw( uno::RuntimeException )
{
    return SwXAutoTextContainer_getImplementationName();
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXAutoTextContainer::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    const uno::Sequence< OUString > aNames = SwXAutoTextContainer_getSupportedServiceNames();
    for(sal_Int32 nService = 0; nService < aNames.getLength(); nService++)
    {
        if(aNames.getConstArray()[nService] == rServiceName)
            return sal_True;
    }
    return sal_False;
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextContainer::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return SwXAutoTextContainer_getSupportedServiceNames();
}
/******************************************************************
 *
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXAutoTextGroup::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
	return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXAutoTextGroup::getSomething( const uno::Sequence< sal_Int8 >& rId )
	throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
										rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
	return 0;
}

/*-- 21.12.98 12:42:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextGroup::SwXAutoTextGroup(const OUString& rName,
			SwGlossaries*	pGlos) :
    pPropSet(aSwMapProvider.GetPropertySet(PROPERTY_MAP_AUTO_TEXT_GROUP)),
    pGlossaries(pGlos),
	sName(rName),
    m_sGroupName(rName)
{
	DBG_ASSERT( -1 != rName.indexOf( GLOS_DELIM ),
		"SwXAutoTextGroup::SwXAutoTextGroup: to be constructed with a complete name only!" );
}

/*-- 21.12.98 12:42:24---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextGroup::~SwXAutoTextGroup()
{
}
/*-- 21.12.98 12:42:24---------------------------------------------------

  -----------------------------------------------------------------------*/
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextGroup::getTitles(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	sal_uInt16 nCount = 0;
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();

	uno::Sequence< OUString > aEntryTitles(nCount);
	OUString *pArr = aEntryTitles.getArray();

	for ( sal_uInt16 i = 0; i < nCount; i++ )
		pArr[i] = pGlosGroup->GetLongName(i);
	delete pGlosGroup;
	return aEntryTitles;
}
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::renameByName(const OUString& aElementName,
	const OUString& aNewElementName, const OUString& aNewElementTitle)
	throw( lang::IllegalArgumentException, container::ElementExistException, io::IOException,
													 uno::RuntimeException)
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	// throw exception only if the programmatic name is to be changed into an existing name
	if(aNewElementName != aElementName && hasByName(aNewElementName))
		throw container::ElementExistException();
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
	{
		sal_uInt16 nIdx = pGlosGroup->GetIndex( aElementName);
		if(USHRT_MAX == nIdx)
			throw lang::IllegalArgumentException();
		String aNewShort( aNewElementName);
		String aNewName( aNewElementTitle);
		sal_uInt16 nOldLongIdx = pGlosGroup->GetLongIndex( aNewShort );
		sal_uInt16 nOldIdx = pGlosGroup->GetIndex( aNewName );

		if( nIdx != USHRT_MAX &&
				(nOldLongIdx == USHRT_MAX || nOldLongIdx == nIdx )&&
					(nOldIdx == USHRT_MAX || nOldIdx == nIdx ))
		{
			pGlosGroup->Rename( nIdx, &aNewShort, &aNewName );
			if(pGlosGroup->GetError() != 0)
				throw io::IOException();
		}
		delete pGlosGroup;
	}
	else
		throw uno::RuntimeException();
}

sal_Bool lcl_CopySelToDoc( SwDoc* pInsDoc, OTextCursorHelper* pxCursor, SwXTextRange* pxRange)
{
	ASSERT( pInsDoc, "kein Ins.Dokument"  );

	SwNodes& rNds = pInsDoc->GetNodes();

	SwNodeIndex aIdx( rNds.GetEndOfContent(), -1 );
	SwCntntNode * pNd = aIdx.GetNode().GetCntntNode();
	SwPosition aPos( aIdx, SwIndex( pNd, pNd->Len() ));

    bool bRet = false;
	pInsDoc->LockExpFlds();
	{
        SwDoc *const pDoc((pxCursor) ? pxCursor->GetDoc() : pxRange->GetDoc());
        SwPaM aPam(pDoc->GetNodes());
        SwPaM * pPam(0);
        if(pxCursor)
        {
            pPam = pxCursor->GetPaM();
        }
        else
        {
            if (pxRange->GetPositions(aPam))
            {
                pPam = & aPam;
            }
        }
        if (!pPam) { return false; }
        bRet = pDoc->CopyRange( *pPam, aPos, false ) || bRet;
	}

	pInsDoc->UnlockExpFlds();
	if( !pInsDoc->IsExpFldsLocked() )
		pInsDoc->UpdateExpFlds(NULL, true);

	return bRet;
}
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XAutoTextEntry >  SwXAutoTextGroup::insertNewByName(const OUString& aName,
		const OUString& aTitle, const uno::Reference< text::XTextRange > & xTextRange)
		throw( container::ElementExistException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	if(hasByName(aName))
		throw container::ElementExistException();
	if(!xTextRange.is())
		throw uno::RuntimeException();

	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
	String sShortName(aName);
	String sLongName(aTitle);
    if(pGlosGroup && !pGlosGroup->GetError())
	{
        /*if( pGlosGroup->IsOld() && pGlosGroup->ConvertToNew())
		{
            throw uno::RuntimeException();
        } */
		uno::Reference<lang::XUnoTunnel> xRangeTunnel( xTextRange, uno::UNO_QUERY);
		SwXTextRange* pxRange = 0;
		OTextCursorHelper* pxCursor = 0;
		if(xRangeTunnel.is())
		{
            pxRange = reinterpret_cast<SwXTextRange*>(xRangeTunnel->getSomething(
                                    SwXTextRange::getUnoTunnelId()));
            pxCursor = reinterpret_cast<OTextCursorHelper*>(xRangeTunnel->getSomething(
                                    OTextCursorHelper::getUnoTunnelId()));
		}

		String sOnlyTxt;
		String* pOnlyTxt = 0;
		sal_Bool bNoAttr = !pxCursor && !pxRange;
		if(bNoAttr)
		{
			sOnlyTxt = UniString(xTextRange->getString());
			pOnlyTxt = &sOnlyTxt;
		}

		const SvxAutoCorrCfg* pCfg = SvxAutoCorrCfg::Get();

		SwDoc* pGDoc = pGlosGroup->GetDoc();

		// Bis es eine Option dafuer gibt, base util::URL loeschen
		if(pCfg->IsSaveRelFile())
		{
            INetURLObject aTemp(pGlosGroup->GetFileName());
            pGlosGroup->SetBaseURL( aTemp.GetMainURL(INetURLObject::NO_DECODE));
		}
		else
            pGlosGroup->SetBaseURL( aEmptyStr );

		sal_uInt16 nRet;
		if( pOnlyTxt )
			nRet = pGlosGroup->PutText( sShortName, sLongName, *pOnlyTxt );
		else
		{
			pGlosGroup->ClearDoc();
			if( pGlosGroup->BeginPutDoc( sShortName, sLongName ) )
			{
				pGDoc->SetRedlineMode_intern( nsRedlineMode_t::REDLINE_DELETE_REDLINES );
				lcl_CopySelToDoc( pGDoc, pxCursor, pxRange );
				pGDoc->SetRedlineMode_intern((RedlineMode_t)( 0 ));
				nRet = pGlosGroup->PutDoc();
			}
			else
				nRet = (sal_uInt16) -1;
		}

		if(nRet == (sal_uInt16) -1 )
		{
			throw uno::RuntimeException();
		}
		pGlossaries->PutGroupDoc( pGlosGroup );
	}

	uno::Reference< text::XAutoTextEntry > xEntry = pGlossaries->GetAutoTextEntry( m_sGroupName, sName, sShortName, true );
	DBG_ASSERT( xEntry.is(), "SwXAutoTextGroup::insertNewByName: no UNO object created? How this?" );
		// we just inserted the entry into the group, so why doesn't it exist?

	return xEntry;
}
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::removeByName(const OUString& aEntryName) throw( container::NoSuchElementException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
	{
		sal_uInt16 nIdx = pGlosGroup->GetIndex(aEntryName);
		if ( nIdx != USHRT_MAX )
			pGlosGroup->Delete(nIdx);
		delete pGlosGroup;
	}
	else
		throw container::NoSuchElementException();
}
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXAutoTextGroup::getName(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	return sName;
}
/*-- 21.12.98 12:42:25---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::setName(const OUString& rName) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	if( !pGlossaries )
		throw uno::RuntimeException();

    sal_Int32 nNewDelimPos = rName.lastIndexOf( GLOS_DELIM );
    sal_Int32 nOldDelimPos = sName.lastIndexOf( GLOS_DELIM );

    OUString aNewSuffix;
    if (nNewDelimPos > -1)
        aNewSuffix = rName.copy( nNewDelimPos + 1 );
    OUString aOldSuffix;
    if (nOldDelimPos > -1)
        aOldSuffix = sName.copy( nOldDelimPos + 1 );

	sal_Int32 nNewNumeric = aNewSuffix.toInt32();
	sal_Int32 nOldNumeric = aOldSuffix.toInt32();

    OUString aNewPrefix( (nNewDelimPos > 1) ? rName.copy( 0, nNewDelimPos ) : rName );
    OUString aOldPrefix( (nOldDelimPos > 1) ? sName.copy( 0, nOldDelimPos ) : sName );

	if ( sName == rName ||
	   ( nNewNumeric == nOldNumeric && aNewPrefix == aOldPrefix ) )
		return;
	String sNewGroup(rName);
	if(STRING_NOTFOUND == sNewGroup.Search(GLOS_DELIM))
	{
		sNewGroup += GLOS_DELIM;
		sNewGroup += UniString::CreateFromInt32(0);
	}

	//the name must be saved, the group may be invalidated while in RenameGroupDoc()
	SwGlossaries* pTempGlossaries = pGlossaries;

	String sPreserveTitle( pGlossaries->GetGroupTitle( sName ) );
	if ( !pGlossaries->RenameGroupDoc( sName, sNewGroup, sPreserveTitle ) )
		throw uno::RuntimeException();
	else
	{
		sName = rName;
		m_sGroupName = sNewGroup;
		pGlossaries = pTempGlossaries;
	}
}
/*-- 21.12.98 12:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int32 SwXAutoTextGroup::getCount(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	int nCount = 0;
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();
	delete pGlosGroup;
	return nCount;
}
/*-- 21.12.98 12:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXAutoTextGroup::getByIndex(sal_Int32 nIndex)
	throw( lang::IndexOutOfBoundsException, lang::WrappedTargetException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	uno::Any aRet;
	sal_uInt16 nCount = 0;
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();
	if(0 <= nIndex && nIndex < nCount)
		aRet = getByName(pGlosGroup->GetShortName((sal_uInt16) nIndex));
	else
		throw lang::IndexOutOfBoundsException();
	delete pGlosGroup;
	return aRet;
}
/*-- 21.12.98 12:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Type SwXAutoTextGroup::getElementType(void) throw( uno::RuntimeException )
{
	return ::getCppuType((uno::Reference<text::XAutoTextEntry>*)0);

}
/*-- 21.12.98 12:42:26---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXAutoTextGroup::hasElements(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
	sal_uInt16 nCount = 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();
	delete pGlosGroup;
	return nCount > 0;

}
/*-- 21.12.98 12:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXAutoTextGroup::getByName(const OUString& _rName)
	throw( container::NoSuchElementException, lang::WrappedTargetException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	uno::Reference< text::XAutoTextEntry > xEntry = pGlossaries->GetAutoTextEntry( m_sGroupName, sName, _rName, true );
	DBG_ASSERT( xEntry.is(), "SwXAutoTextGroup::getByName: GetAutoTextEntry is fractious!" );
		// we told it to create the object, so why didn't it?
	return makeAny( xEntry );
}
/*-- 21.12.98 12:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextGroup::getElementNames(void)
	throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	sal_uInt16 nCount = 0;
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();

	uno::Sequence< OUString > aEntryNames(nCount);
	OUString *pArr = aEntryNames.getArray();

	for ( sal_uInt16 i = 0; i < nCount; i++ )
		pArr[i] = pGlosGroup->GetShortName(i);
	delete pGlosGroup;
	return aEntryNames;
}
/*-- 21.12.98 12:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SwXAutoTextGroup::hasByName(const OUString& rName)
	throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	sal_Bool bRet = sal_False;
	sal_uInt16 nCount = 0;
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(pGlosGroup && !pGlosGroup->GetError())
		nCount = pGlosGroup->GetCount();
	else
		throw uno::RuntimeException();

	for( sal_uInt16 i = 0; i < nCount; i++ )
	{
		String sCompare(pGlosGroup->GetShortName(i));
		if(COMPARE_EQUAL == sCompare.CompareIgnoreCaseToAscii(String(rName)))
		{
			bRet = sal_True;
			break;
		}
	}
	delete pGlosGroup;
	return bRet;
}

/*-- 09.02.00 15:33:30---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SwXAutoTextGroup::getPropertySetInfo(void)
	throw( uno::RuntimeException )
{
    static uno::Reference< beans::XPropertySetInfo >  xRet = pPropSet->getPropertySetInfo();
	return xRet;
}
/*-- 09.02.00 15:33:31---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::setPropertyValue(
	const OUString& rPropertyName, const uno::Any& aValue)
	throw( beans::UnknownPropertyException, beans::PropertyVetoException,
 		lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
    const SfxItemPropertySimpleEntry*   pEntry = pPropSet->getPropertyMap()->getByName( rPropertyName );

    if(!pEntry)
		throw beans::UnknownPropertyException();

	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(!pGlosGroup || pGlosGroup->GetError())
		throw uno::RuntimeException();
    switch(pEntry->nWID)
	{
		case  WID_GROUP_TITLE:
		{
            OUString sNewTitle;
            aValue >>= sNewTitle;
            if(!sNewTitle.getLength())
				throw lang::IllegalArgumentException();
            sal_Bool bChanged = !sNewTitle.equals(pGlosGroup->GetName());
			pGlosGroup->SetName(sNewTitle);
			if(bChanged && HasGlossaryList())
				GetGlossaryList()->ClearGroups();
		}
		break;
	}
	delete pGlosGroup;
}
/*-- 09.02.00 15:33:31---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SwXAutoTextGroup::getPropertyValue(const OUString& rPropertyName)
	throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
    const SfxItemPropertySimpleEntry*   pEntry = pPropSet->getPropertyMap()->getByName( rPropertyName);

    if(!pEntry)
		throw beans::UnknownPropertyException();
	SwTextBlocks* pGlosGroup = pGlossaries ? pGlossaries->GetGroupDoc(m_sGroupName, sal_False) : 0;
    if(!pGlosGroup  || pGlosGroup->GetError())
		throw uno::RuntimeException();

	uno::Any aAny;
    switch(pEntry->nWID)
	{
		case  WID_GROUP_PATH:
			aAny <<= OUString(pGlosGroup->GetFileName());
		break;
		case  WID_GROUP_TITLE:
			aAny <<= OUString(pGlosGroup->GetName());
		break;
	}
	delete pGlosGroup;
	return aAny;
}
/*-- 09.02.00 15:33:31---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::addPropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
	throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 09.02.00 15:33:31---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::removePropertyChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XPropertyChangeListener > & /*aListener*/)
	throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 09.02.00 15:33:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::addVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
	throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 09.02.00 15:33:32---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::removeVetoableChangeListener(
    const OUString& /*PropertyName*/, const uno::Reference< beans::XVetoableChangeListener > & /*aListener*/)
	throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 21.12.98 12:42:27---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextGroup::Invalidate()
{
	pGlossaries = 0;
	sName = aEmptyStr;
	m_sGroupName = aEmptyStr;
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXAutoTextGroup::getImplementationName(void) throw( uno::RuntimeException )
{
	return C2U("SwXAutoTextGroup");
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXAutoTextGroup::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
	return C2U("com.sun.star.text.AutoTextGroup") == rServiceName;
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextGroup::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
	uno::Sequence< OUString > aRet(1);
	OUString* pArray = aRet.getArray();
	pArray[0] = C2U("com.sun.star.text.AutoTextGroup");
	return aRet;
}
/******************************************************************
 *
 ******************************************************************/
/* -----------------------------10.03.00 18:02--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SwXAutoTextEntry::getUnoTunnelId()
{
    static uno::Sequence< sal_Int8 > aSeq = ::CreateUnoTunnelId();
	return aSeq;
}
/* -----------------------------10.03.00 18:04--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SwXAutoTextEntry::getSomething( const uno::Sequence< sal_Int8 >& rId )
	throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
										rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >( reinterpret_cast< sal_IntPtr >( this ));
    }
	return 0;
}
/*-- 21.12.98 12:42:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextEntry::SwXAutoTextEntry(SwGlossaries* pGlss, const String& rGroupName,
											const String& rEntryName) :
	pGlossaries(pGlss),
	sGroupName(rGroupName),
	sEntryName(rEntryName),
	pBodyText ( NULL )
{
}
/*-- 21.12.98 12:42:33---------------------------------------------------

  -----------------------------------------------------------------------*/
SwXAutoTextEntry::~SwXAutoTextEntry()
{
	{
	    ::vos::OGuard aGuard(Application::GetSolarMutex());

		// ensure that any pending modifications are written
		implFlushDocument( true );

		//! Bug #96559
		// DocShell must be cleared before mutex is lost.
		// Needs to be done explicitly since xDocSh is a class member.
		// Thus, an own block here, guarded by the SolarMutex
	}
}

//---------------------------------------------------------------------
//--- 03.03.2003 13:24:58 -----------------------------------------------

void SwXAutoTextEntry::implFlushDocument( bool _bCloseDoc )
{
	if ( xDocSh.Is() )
	{
		if ( xDocSh->GetDoc()->IsModified () )
			xDocSh->Save();

        if ( _bCloseDoc )
		{
			// stop listening at the document
			EndListening( *&xDocSh );

			xDocSh->DoClose();
			xDocSh.Clear();
		}
	}
}

//-----------------------------------------------------------------------
//--- 03.03.2003 15:51:52 -----------------------------------------------

void SwXAutoTextEntry::Notify( SfxBroadcaster& _rBC, const SfxHint& _rHint )
{
	if ( &_rBC == &xDocSh )
	{	// it's our document
		if ( _rHint.ISA( SfxSimpleHint ) )
		{
            if ( SFX_HINT_DEINITIALIZING == static_cast< const SfxSimpleHint& >( _rHint ).GetId() )
			{
				// our document is dying (possibly because we're shutting down, and the document was notified
				// earlier than we are?)
				// stop listening at the docu
				EndListening( *&xDocSh );
				// and release our reference
				xDocSh.Clear();
			}
		}
        else if(_rHint.ISA(SfxEventHint))
        {
            if(SFX_EVENT_PREPARECLOSEDOC == static_cast< const SfxEventHint& >( _rHint ).GetEventId())
            {
                implFlushDocument( sal_False );
                xBodyText = 0;
                xDocSh.Clear();
            }
        }
	}
}

void SwXAutoTextEntry::GetBodyText ()
{
	::vos::OGuard aGuard(Application::GetSolarMutex());

	xDocSh = pGlossaries->EditGroupDoc ( sGroupName, sEntryName, sal_False );
	DBG_ASSERT( xDocSh.Is(), "SwXAutoTextEntry::GetBodyText: unexpected: no doc returned by EditGroupDoc!" );

	// start listening at the document
	StartListening( *&xDocSh );

	pBodyText = new SwXBodyText ( xDocSh->GetDoc() );
	xBodyText = uno::Reference < lang::XServiceInfo > ( *pBodyText, uno::UNO_QUERY);
}

uno::Reference< text::XTextCursor >  SwXAutoTextEntry::createTextCursor(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	return pBodyText->createTextCursor();
}
/*-- 21.12.98 12:42:34---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextCursor >  SwXAutoTextEntry::createTextCursorByRange(
	const uno::Reference< text::XTextRange > & aTextPosition) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	return pBodyText->createTextCursorByRange ( aTextPosition );
}
/*-- 21.12.98 12:42:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextEntry::insertString(const uno::Reference< text::XTextRange > & xRange, const OUString& aString, sal_Bool bAbsorb) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	pBodyText->insertString ( xRange, aString, bAbsorb );
}
/*-- 21.12.98 12:42:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextEntry::insertControlCharacter(const uno::Reference< text::XTextRange > & xRange,
	sal_Int16 nControlCharacter, sal_Bool bAbsorb)
		throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	pBodyText->insertControlCharacter ( xRange, nControlCharacter, bAbsorb );
}
/*-- 21.12.98 12:42:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextEntry::insertTextContent(
	const uno::Reference< text::XTextRange > & xRange,
	const uno::Reference< text::XTextContent > & xContent, sal_Bool bAbsorb)
		throw( lang::IllegalArgumentException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	pBodyText->insertTextContent ( xRange, xContent, bAbsorb );
}
/*-- 21.12.98 12:42:34---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextEntry::removeTextContent(
	const uno::Reference< text::XTextContent > & xContent)
		throw( container::NoSuchElementException, uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	pBodyText->removeTextContent ( xContent );
}
/*-- 21.12.98 12:42:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XText >  SwXAutoTextEntry::getText(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	uno::Reference< text::XText >  xRet =  (text::XText*)this;
	return xRet;
}
/*-- 21.12.98 12:42:35---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXAutoTextEntry::getStart(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	return pBodyText->getStart();
}
/*-- 21.12.98 12:42:36---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< text::XTextRange >  SwXAutoTextEntry::getEnd(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	return pBodyText->getEnd();
}
/*-- 21.12.98 12:42:36---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SwXAutoTextEntry::getString(void) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	return pBodyText->getString();
}
/*-- 21.12.98 12:42:36---------------------------------------------------

  -----------------------------------------------------------------------*/
void SwXAutoTextEntry::setString(const OUString& aString) throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());
	EnsureBodyText();
	pBodyText->setString( aString );
}
/* -----------------15.07.99 10:11-------------------

 --------------------------------------------------*/
void SwXAutoTextEntry::applyTo(const uno::Reference< text::XTextRange > & xTextRange)throw( uno::RuntimeException )
{
	::vos::OGuard aGuard(Application::GetSolarMutex());

	// ensure that any pending modifications are written
	// reason is that we're holding the _copy_ of the auto text, while the real auto text
	// is stored somewhere. And below, we're not working with our copy, but only tell the target
	// TextRange to work with the stored version.
	// #96380# - 2003-03-03 - fs@openoffice.org
	implFlushDocument( false );
		// TODO: think about if we should pass "true" here
		// The difference would be that when the next modification is made to this instance here, then
		// we would be forced to open the document again, instead of working on our current copy.
		// This means that we would reflect any changes which were done to the AutoText by foreign instances
		// in the meantime

	uno::Reference<lang::XUnoTunnel> xTunnel( xTextRange, uno::UNO_QUERY);
	SwXTextRange* pRange = 0;
	OTextCursorHelper* pCursor = 0;
	SwXText *pText = 0;

	if(xTunnel.is())
	{
		pRange = reinterpret_cast < SwXTextRange* >
				( xTunnel->getSomething( SwXTextRange::getUnoTunnelId() ) );
		pCursor = reinterpret_cast < OTextCursorHelper*>
				( xTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() ) );
		pText = reinterpret_cast < SwXText* >
				( xTunnel->getSomething( SwXText::getUnoTunnelId() ) );
	}

	SwDoc* pDoc = 0;
    if (pRange)
		pDoc = pRange->GetDoc();
	else if ( pCursor )
		pDoc = pCursor->GetDoc();
	else if ( pText && pText->GetDoc() )
	{
		xTunnel = uno::Reference < lang::XUnoTunnel > (pText->getStart(), uno::UNO_QUERY);
		if (xTunnel.is())
		{
			pCursor = reinterpret_cast < OTextCursorHelper* >
				( xTunnel->getSomething( OTextCursorHelper::getUnoTunnelId() ) );
			if (pCursor)
				pDoc = pText->GetDoc();
		}
	}

	if(!pDoc)
		throw uno::RuntimeException();

    SwPaM InsertPaM(pDoc->GetNodes());
    if (pRange)
    {
        if (!pRange->GetPositions(InsertPaM))
        {
            throw uno::RuntimeException();
        }
    }
    else
    {
        InsertPaM = *pCursor->GetPaM();
    }

    ::std::auto_ptr<SwTextBlocks> pBlock(pGlossaries->GetGroupDoc(sGroupName));
    const bool bResult = pBlock.get() && !pBlock->GetError()
                    && pDoc->InsertGlossary( *pBlock, sEntryName, InsertPaM);

	if(!bResult)
		throw uno::RuntimeException();
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwXAutoTextEntry::getImplementationName(void) throw( uno::RuntimeException )
{
	return C2U("SwXAutoTextEntry");
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
sal_Bool SwXAutoTextEntry::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
	return C2U("com.sun.star.text.AutoTextEntry") == rServiceName;
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Sequence< OUString > SwXAutoTextEntry::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
	uno::Sequence< OUString > aRet(1);
	OUString* pArray = aRet.getArray();
	pArray[0] = C2U("com.sun.star.text.AutoTextEntry");
	return aRet;
}
/* -----------------------------06.04.00 11:11--------------------------------

 ---------------------------------------------------------------------------*/
uno::Reference< container::XNameReplace > SwXAutoTextEntry::getEvents()
	throw( uno::RuntimeException )
{
	return new SwAutoTextEventDescriptor( *this );
}
/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
const struct SvEventDescription aAutotextEvents[] =
{
	{ SW_EVENT_START_INS_GLOSSARY,	"OnInsertStart" },
	{ SW_EVENT_END_INS_GLOSSARY,	"OnInsertDone" },
	{ 0, NULL }
};

/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
SwAutoTextEventDescriptor::SwAutoTextEventDescriptor(
	SwXAutoTextEntry& rAutoText ) :
		SvBaseEventDescriptor(aAutotextEvents),
		sSwAutoTextEventDescriptor(RTL_CONSTASCII_USTRINGPARAM(
			"SwAutoTextEventDescriptor")),
		rAutoTextEntry(rAutoText)
{
}
/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
SwAutoTextEventDescriptor::~SwAutoTextEventDescriptor()
{
}
/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
OUString SwAutoTextEventDescriptor::getImplementationName()
	throw( uno::RuntimeException )
{
	return sSwAutoTextEventDescriptor;
}
/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoTextEventDescriptor::replaceByName(
	const sal_uInt16 nEvent,
	const SvxMacro& rMacro)
			throw(
				lang::IllegalArgumentException,
				container::NoSuchElementException,
				lang::WrappedTargetException,
				uno::RuntimeException)
{
	DBG_ASSERT( NULL != rAutoTextEntry.GetGlossaries(),
				"Strangely enough, the AutoText vanished!" );
	DBG_ASSERT( (nEvent == SW_EVENT_END_INS_GLOSSARY) ||
				(nEvent == SW_EVENT_START_INS_GLOSSARY) ,
				"Unknown event ID" );

	const SwGlossaries* pGlossaries = rAutoTextEntry.GetGlossaries();
	SwTextBlocks* pBlocks =
		pGlossaries->GetGroupDoc( rAutoTextEntry.GetGroupName() );
	DBG_ASSERT( NULL != pBlocks,
			    "can't get autotext group; SwAutoTextEntry has illegal name?");

    if( pBlocks && !pBlocks->GetError())
	{
		sal_uInt16 nIndex = pBlocks->GetIndex( rAutoTextEntry.GetEntryName() );
		if( nIndex != USHRT_MAX )
		{
			SvxMacroTableDtor aMacroTable;
			if( pBlocks->GetMacroTable( nIndex, aMacroTable ) )
			{
				SvxMacro* pNewMacro = new SvxMacro(rMacro);
				aMacroTable.Replace( nEvent, pNewMacro );
				pBlocks->SetMacroTable( nIndex, aMacroTable );
			}
		}

		delete pBlocks;
	}
	// else: ignore
}
/* -----------------------------30.01.01 18:40--------------------------------

 ---------------------------------------------------------------------------*/
void SwAutoTextEventDescriptor::getByName(
	SvxMacro& rMacro,
	const sal_uInt16 nEvent )
			throw(
				container::NoSuchElementException,
				lang::WrappedTargetException,
				uno::RuntimeException)
{
	DBG_ASSERT( NULL != rAutoTextEntry.GetGlossaries(), "no AutoText" );
	DBG_ASSERT( (nEvent == SW_EVENT_END_INS_GLOSSARY) ||
				(nEvent == SW_EVENT_START_INS_GLOSSARY) ,
				"Unknown event ID" );

	const SwGlossaries* pGlossaries = rAutoTextEntry.GetGlossaries();
	SwTextBlocks* pBlocks =
		pGlossaries->GetGroupDoc( rAutoTextEntry.GetGroupName() );
	DBG_ASSERT( NULL != pBlocks,
			    "can't get autotext group; SwAutoTextEntry has illegal name?");

	// return empty macro, unless macro is found
    OUString sEmptyStr;
    SvxMacro aEmptyMacro(sEmptyStr, sEmptyStr);
	rMacro = aEmptyMacro;

    if ( pBlocks &&  !pBlocks->GetError())
	{
		sal_uInt16 nIndex = pBlocks->GetIndex( rAutoTextEntry.GetEntryName() );
		if( nIndex != USHRT_MAX )
		{
			SvxMacroTableDtor aMacroTable;
			if( pBlocks->GetMacroTable( nIndex, aMacroTable ) )
			{
				SvxMacro *pMacro = aMacroTable.Get( nEvent );
				if( pMacro )
					rMacro = *pMacro;
			}
		}

		delete pBlocks;
	}
}




