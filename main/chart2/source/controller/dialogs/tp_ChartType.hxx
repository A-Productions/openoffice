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



#ifndef _CHART2_TP_CHARTTYPE_HXX
#define _CHART2_TP_CHARTTYPE_HXX

#include <vector>

#include "ChartTypeDialogController.hxx"
#include "ChartTypeTemplateProvider.hxx"
#include "TimerTriggeredControllerLock.hxx"

#include <com/sun/star/chart2/XChartDocument.hpp>
#include <svtools/wizardmachine.hxx>
#include <svtools/valueset.hxx>
#include <vcl/fixed.hxx>
// header for class MetricField
#include <vcl/field.hxx>
// header for class ListBox
#include <vcl/lstbox.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>

//.............................................................................
namespace chart
{
//.............................................................................

//-----------------------------------------------------------------------------
/**
*/
class ChartTypeResourceGroup;
class AxisTypeResourceGroup;
class Dim3DLookResourceGroup;
class StackingResourceGroup;
class SplineResourceGroup;
class GeometryResourceGroup;
class ChartTypeParameter;
class SortByXValuesResourceGroup;

class ChartTypeTabPage : public ResourceChangeListener, public svt::OWizardPage, public ChartTypeTemplateProvider
{
public:
    ChartTypeTabPage( Window* pParent
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartDocument >& xChartModel
                , const ::com::sun::star::uno::Reference<
                ::com::sun::star::uno::XComponentContext >& xContext
                , bool bDoLiveUpdate, bool bHideDescription = false );
    virtual ~ChartTypeTabPage();

    virtual void		initializePage();
    virtual sal_Bool	commitPage( ::svt::WizardTypes::CommitPageReason eReason );

    virtual ::com::sun::star::uno::Reference<
                ::com::sun::star::chart2::XChartTypeTemplate > getCurrentTemplate() const;

protected:
    ChartTypeDialogController* getSelectedMainType();
    void showAllControls( ChartTypeDialogController& rTypeController );
    void fillAllControls( const ChartTypeParameter& rParameter, bool bAlsoResetSubTypeList=true );
    ChartTypeParameter getCurrentParamter() const;
    
    virtual void stateChanged( ChangingResource* pResource );

    void commitToModel( const ChartTypeParameter& rParameter );

    DECL_LINK( SelectMainTypeHdl, void* );
    DECL_LINK( SelectSubTypeHdl, void* );

protected:
    FixedText   m_aFT_ChooseType;
    ListBox     m_aMainTypeList;
    ValueSet    m_aSubTypeList;

    AxisTypeResourceGroup*      m_pAxisTypeResourceGroup;
    Dim3DLookResourceGroup*     m_pDim3DLookResourceGroup;
    StackingResourceGroup*      m_pStackingResourceGroup;
    SplineResourceGroup*        m_pSplineResourceGroup;
    GeometryResourceGroup*      m_pGeometryResourceGroup;
    SortByXValuesResourceGroup* m_pSortByXValuesResourceGroup;

    long    m_nYTopPos;//top y position of the different ResourceGroups (AxisTypeResourceGroup, etc)

    ::com::sun::star::uno::Reference<
                       ::com::sun::star::chart2::XChartDocument >   m_xChartModel;
    ::com::sun::star::uno::Reference<
                       ::com::sun::star::uno::XComponentContext >    m_xCC;

    ::std::vector< ChartTypeDialogController* > m_aChartTypeDialogControllerList;
    ChartTypeDialogController*                  m_pCurrentMainType;

    sal_Int32 m_nChangingCalls;
    bool      m_bDoLiveUpdate;

    TimerTriggeredControllerLock   m_aTimerTriggeredControllerLock;
};

//.............................................................................
} //namespace chart
//.............................................................................
#endif
