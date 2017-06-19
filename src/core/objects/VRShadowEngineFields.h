#ifndef VRSHADOWENGINEFIELDS_H_INCLUDED
#define VRSHADOWENGINEFIELDS_H_INCLUDED

/*---------------------------------------------------------------------------*\
 *                                OpenSG                                     *
 *                                                                           *
 *                                                                           *
 *               Copyright (C) 2000-2013 by the OpenSG Forum                 *
 *                                                                           *
 *                            www.opensg.org                                 *
 *                                                                           *
 * contact: dirk@opensg.org, gerrit.voss@vossg.org, carsten_neumann@gmx.net  *
 *                                                                           *
\*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*\
 *                                License                                    *
 *                                                                           *
 * This library is free software; you can redistribute it and/or modify it   *
 * under the terms of the GNU Library General Public License as published    *
 * by the Free Software Foundation, version 2.                               *
 *                                                                           *
 * This library is distributed in the hope that it will be useful, but       *
 * WITHOUT ANY WARRANTY; without even the implied warranty of                *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU         *
 * Library General Public License for more details.                          *
 *                                                                           *
 * You should have received a copy of the GNU Library General Public         *
 * License along with this library; if not, write to the Free Software       *
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                 *
 *                                                                           *
\*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*\
 *                                Changes                                    *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *                                                                           *
 *                                                                           *
\*---------------------------------------------------------------------------*/

/*****************************************************************************\
 *****************************************************************************
 **                                                                         **
 **                  This file is automatically generated.                  **
 **                                                                         **
 **          Any changes made to this file WILL be lost when it is          **
 **           regenerated, which can become necessary at any time.          **
 **                                                                         **
 *****************************************************************************
\*****************************************************************************/


#include <OpenSG/OSGConfig.h>
#include <OpenSG/OSGGroupDef.h>

#include <OpenSG/OSGFieldContainerFields.h>
#include <OpenSG/OSGPointerSField.h>
#include <OpenSG/OSGPointerMField.h>


OSG_BEGIN_NAMESPACE


class VRShadowEngine;

OSG_GEN_CONTAINERPTR(VRShadowEngine);
/*! \ingroup GrpGroupLightShadowEnginesFieldTraits
    \ingroup GrpLibOSGGroup
 */
template <>
struct FieldTraits<VRShadowEngine *, nsOSG> :
    public FieldTraitsFCPtrBase<VRShadowEngine *, nsOSG>
{
  private:

    static PointerType             _type;

  public:

    typedef FieldTraits<VRShadowEngine *, nsOSG>  Self;

    enum                        { Convertible = NotConvertible };

    static OSG_GROUP_DLLMAPPING DataType &getType(void);

    template<typename RefCountPolicy> inline
    static const Char8    *getSName     (void);

    template<typename RefCountPolicy> inline
    static const Char8    *getMName     (void);

};

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getSName<RecordedRefCountPolicy>(void)
{
    return "SFRecVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getSName<UnrecordedRefCountPolicy>(void)
{
    return "SFUnrecVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getSName<WeakRefCountPolicy>(void)
{
    return "SFWeakVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getSName<NoRefCountPolicy>(void)
{
    return "SFUnrefdVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getMName<RecordedRefCountPolicy>(void)
{
    return "MFRecVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getMName<UnrecordedRefCountPolicy>(void)
{
    return "MFUnrecVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getMName<WeakRefCountPolicy>(void)
{
    return "MFWeakVRShadowEnginePtr";
}

template<> inline
const Char8 *FieldTraits<VRShadowEngine *, nsOSG>::getMName<NoRefCountPolicy>(void)
{
    return "MFUnrefdVRShadowEnginePtr";
}


#ifndef DOXYGEN_SHOULD_SKIP_THIS
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields */
typedef PointerSField<VRShadowEngine *,
                      RecordedRefCountPolicy, nsOSG  > SFRecVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields */
typedef PointerSField<VRShadowEngine *,
                      UnrecordedRefCountPolicy, nsOSG> SFUnrecVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields */
typedef PointerSField<VRShadowEngine *,
                      WeakRefCountPolicy, nsOSG      > SFWeakVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields */
typedef PointerSField<VRShadowEngine *,
                      NoRefCountPolicy, nsOSG        > SFUncountedVRShadowEnginePtr;


/*! \ingroup GrpGroupLightShadowEnginesFieldMFields */
typedef PointerMField<VRShadowEngine *,
                      RecordedRefCountPolicy, nsOSG  > MFRecVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields */
typedef PointerMField<VRShadowEngine *,
                      UnrecordedRefCountPolicy, nsOSG> MFUnrecVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields */
typedef PointerMField<VRShadowEngine *,
                      WeakRefCountPolicy, nsOSG      > MFWeakVRShadowEnginePtr;
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields */
typedef PointerMField<VRShadowEngine *,
                      NoRefCountPolicy, nsOSG        > MFUncountedVRShadowEnginePtr;




#else // these are the doxygen hacks

/*! \ingroup GrpGroupLightShadowEnginesFieldSFields \ingroup GrpLibOSGGroup */
struct SFRecVRShadowEnginePtr :
    public PointerSField<VRShadowEngine *,
                         RecordedRefCountPolicy> {};
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields \ingroup GrpLibOSGGroup */
struct SFUnrecVRShadowEnginePtr :
    public PointerSField<VRShadowEngine *,
                         UnrecordedRefCountPolicy> {};
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields \ingroup GrpLibOSGGroup */
struct SFWeakVRShadowEnginePtr :
    public PointerSField<VRShadowEngine *,
                         WeakRefCountPolicy> {};
/*! \ingroup GrpGroupLightShadowEnginesFieldSFields \ingroup GrpLibOSGGroup */
struct SFUncountedVRShadowEnginePtr :
    public PointerSField<VRShadowEngine *,
                         NoRefCountPolicy> {};


/*! \ingroup GrpGroupLightShadowEnginesFieldMFields \ingroup GrpLibOSGGroup */
struct MFRecVRShadowEnginePtr :
    public PointerMField<VRShadowEngine *,
                         RecordedRefCountPolicy  > {};
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields \ingroup GrpLibOSGGroup */
struct MFUnrecVRShadowEnginePtr :
    public PointerMField<VRShadowEngine *,
                         UnrecordedRefCountPolicy> {};
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields \ingroup GrpLibOSGGroup */
struct MFWeakVRShadowEnginePtr :
    public PointerMField<VRShadowEngine *,
                         WeakRefCountPolicy      > {};
/*! \ingroup GrpGroupLightShadowEnginesFieldMFields \ingroup GrpLibOSGGroup */
struct MFUncountedVRShadowEnginePtr :
    public PointerMField<VRShadowEngine *,
                         NoRefCountPolicy        > {};



#endif // these are the doxygen hacks

OSG_END_NAMESPACE

#endif // VRSHADOWENGINEFIELDS_H_INCLUDED