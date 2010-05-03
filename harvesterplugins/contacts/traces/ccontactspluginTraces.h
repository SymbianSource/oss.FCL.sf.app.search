// Created by TraceCompiler 2.1.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CCONTACTSPLUGINTRACES_H__
#define __CCONTACTSPLUGINTRACES_H__

#define KOstTraceComponentID 0x2001f702

#define DUP3_CCONTACTSPLUGIN_HANDLEDATABASEEVENTL 0x860001
#define CCONTACTSPLUGIN_HANDLEDATABASEEVENTL 0x860002
#define DUP1_CCONTACTSPLUGIN_HANDLEDATABASEEVENTL 0x860003
#define CCONTACTSPLUGIN_DELAYEDCALLBACKL 0x860004
#define CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x860005
#define DUP1_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x860006
#define DUP2_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x860007
#define DUP3_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x860008
#define DUP4_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x860009
#define DUP5_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x86000a
#define DUP6_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x86000b
#define DUP7_CCONTACTSPLUGIN_CREATECONTACTINDEXITEML 0x86000c


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen1( TUint32 aTraceID, const TDesC16& aParam1 )
    {
    TBool retval;
    TInt size = aParam1.Size();
    // BTrace assumes that parameter size is atleast 4 bytes
    if (size % 4 == 0)
        {
        TUint8* ptr = ( TUint8* )aParam1.Ptr();
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    else
        {
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        if (size > KOstMaxDataLength)
            {
            size = KOstMaxDataLength;
            }
        TInt sizeAligned = ( size + 3 ) & ~3;
        memcpy( ptr, aParam1.Ptr(), size );
        ptr += size;
        // Fillers are written to get 32-bit alignment
        while ( size++ < sizeAligned )
            {
            *ptr++ = 0;
            }
        ptr -= sizeAligned;
        size = sizeAligned;
        // Data is written directly and length is determined from trace message length
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, size );
        }
    return retval;
    }
#endif


#endif

// End of file

