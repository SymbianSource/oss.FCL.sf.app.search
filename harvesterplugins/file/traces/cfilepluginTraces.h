// Created by TraceCompiler 2.1.2
// DO NOT EDIT, CHANGES WILL BE LOST

#ifndef __CFILEPLUGINTRACES_H__
#define __CFILEPLUGINTRACES_H__

#define KOstTraceComponentID 0x2001f703

#define CFILEPLUGIN_MOUNTL_ENTRY 0x8a0014
#define CFILEPLUGIN_MOUNTL_EXIT 0x8a0015
#define CFILEPLUGIN_UNMOUNT_ENTRY 0x8a0016
#define CFILEPLUGIN_UNMOUNT_EXIT 0x8a0017
#define DUP1_CFILEPLUGIN_UNMOUNT_EXIT 0x8a0018
#define CFILEPLUGIN_STARTHARVESTINGL_ENTRY 0x8a0019
#define CFILEPLUGIN_STARTHARVESTINGL_EXIT 0x8a001a
#define CFILEPLUGIN_HARVESTINGCOMPLETED_ENTRY 0x8a001b
#define CFILEPLUGIN_HARVESTINGCOMPLETED_EXIT 0x8a001c
#define CFILEPLUGIN_ADDNOTIFICATIONPATHSL_ENTRY 0x8a001d
#define CFILEPLUGIN_ADDNOTIFICATIONPATHSL_EXIT 0x8a001e
#define CFILEPLUGIN_REMOVENOTIFICATIONPATHS_ENTRY 0x8a001f
#define CFILEPLUGIN_REMOVENOTIFICATIONPATHS_EXIT 0x8a0020
#define CFILEPLUGIN_FORMBASEAPPCLASS_ENTRY 0x8a0021
#define CFILEPLUGIN_FORMBASEAPPCLASS_EXIT 0x8a0022
#define CFILEPLUGIN_DATABASEPATHLC_ENTRY 0x8a0023
#define CFILEPLUGIN_DATABASEPATHLC_EXIT 0x8a0024
#define CFILEPLUGIN_CONSTRUCTL 0x860029
#define DUP1_CFILEPLUGIN_CONSTRUCTL 0x86002a
#define DUP2_CFILEPLUGIN_CONSTRUCTL 0x86002b
#define DUP3_CFILEPLUGIN_CONSTRUCTL 0x86002c
#define DUP4_CFILEPLUGIN_CONSTRUCTL 0x86002d
#define DUP5_CFILEPLUGIN_CONSTRUCTL 0x86002e
#define CFILEPLUGIN_STARTPLUGINL 0x86002f
#define DUP1_CFILEPLUGIN_STARTPLUGINL 0x860030
#define DUP2_CFILEPLUGIN_STARTPLUGINL 0x860031
#define DUP1_CFILEPLUGIN_CREATECONTENTINDEXITEML 0x860032
#define DUP2_CFILEPLUGIN_CREATECONTENTINDEXITEML 0x860033
#define CFILEPLUGIN_CREATECONTENTINDEXITEML 0x860034
#define DUP3_CFILEPLUGIN_CREATECONTENTINDEXITEML 0x860035
#define DUP4_CFILEPLUGIN_CREATECONTENTINDEXITEML 0x860036
#define CFILEPLUGIN_CREATEFOLDERFILEINDEXITEML 0x860037
#define DUP1_CFILEPLUGIN_CREATEFOLDERFILEINDEXITEML 0x860038
#define DUP2_CFILEPLUGIN_CREATEFOLDERFILEINDEXITEML 0x860039
#define DUP3_CFILEPLUGIN_CREATEFOLDERFILEINDEXITEML 0x86003a
#define DUP4_CFILEPLUGIN_CREATEFOLDERFILEINDEXITEML 0x86003b


#ifndef __KERNEL_MODE__
inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC16& aParam1, TInt aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        // Check that there are enough space to next parameter
        if ((length + sizeof ( TInt )) <= KOstMaxDataLength)
            {
            *( ( TInt* )ptr ) = aParam2;
            ptr += sizeof ( TInt );
            length += sizeof ( TInt );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }

inline TBool OstTraceGen2( TUint32 aTraceID, const TDesC16& aParam1, TInt32 aParam2 )
    {
    TBool retval = BTraceFiltered8( EXTRACT_GROUP_ID(aTraceID), EOstTraceActivationQuery, KOstTraceComponentID, aTraceID );
    if ( retval )
        {
        TInt length = 0;
        // Check that parameter lenght is not too long
        TInt length1 = aParam1.Size();
        if ((length + length1 + sizeof ( TUint32 )) > KOstMaxDataLength)
            {
            length1 = KOstMaxDataLength - (length + sizeof ( TUint32 ));
            }
        TInt lengthAligned1 = ( length1 + 3 ) & ~3;
        if (lengthAligned1 > 0)
            {
            length = length + sizeof ( TUint32 ) + lengthAligned1;
            }
        TUint8 data[ KOstMaxDataLength ];
        TUint8* ptr = data;
        // Set length to zero and calculate it againg
        // when adding parameters
        length = 0;
        if (length1 > 0)
            {
            // Number of elements is written before data
            // In case of Unicode string, number of elements is half of length
            *( ( TUint32* )ptr ) = length1 / (aParam1.Size() / aParam1.Length());
            ptr += sizeof ( TUint32 );
            memcpy( ptr, aParam1.Ptr(), length1 );
            ptr += length1;
            // Fillers are written to get 32-bit alignment
            while ( length1++ < lengthAligned1 )
                {
                *ptr++ = 0;
                }
            length += sizeof ( TUint32 ) + lengthAligned1;
            }
        else if (length + sizeof ( TUint32 ) <= KOstMaxDataLength)
            {
            *( ( TUint32* )ptr ) = 0;
            ptr += sizeof ( TUint32 );
            length += sizeof ( TUint32 );
            }
        // Check that there are enough space to next parameter
        if ((length + sizeof ( TInt )) <= KOstMaxDataLength)
            {
            *( ( TInt* )ptr ) = aParam2;
            ptr += sizeof ( TInt );
            length += sizeof ( TInt );
            }
        ptr -= length;
        retval = OstSendNBytes( EXTRACT_GROUP_ID(aTraceID), EOstTrace, KOstTraceComponentID, aTraceID, ptr, length );
        }
    return retval;
    }
#endif


#endif

// End of file

