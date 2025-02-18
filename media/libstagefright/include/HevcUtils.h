/*
 * Copyright (C) 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HEVC_UTILS_H_

#define HEVC_UTILS_H_

#include <stdint.h>

#include <media/stagefright/foundation/ABase.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ABitReader.h>
#include <utils/Errors.h>
#include <utils/KeyedVector.h>
#include <utils/StrongPointer.h>
#include <utils/Vector.h>

namespace android {

enum {
    kHevcNalUnitTypeCodedSliceIdr = 19,
    kHevcNalUnitTypeCodedSliceIdrNoLP = 20,
    kHevcNalUnitTypeCodedSliceCra = 21,

    kHevcNalUnitTypeVps = 32,
    kHevcNalUnitTypeSps = 33,
    kHevcNalUnitTypePps = 34,
    kHevcNalUnitTypePrefixSei = 39,
    kHevcNalUnitTypeSuffixSei = 40,
};

enum {
    // uint8_t
    kGeneralProfileSpace,
    // uint8_t
    kGeneralTierFlag,
    // uint8_t
    kGeneralProfileIdc,
    // uint32_t
    kGeneralProfileCompatibilityFlags,
    // uint64_t
    kGeneralConstraintIndicatorFlags,
    // uint8_t
    kGeneralLevelIdc,
    // uint8_t
    kChromaFormatIdc,
    // uint8_t
    kBitDepthLumaMinus8,
    // uint8_t
    kBitDepthChromaMinus8,
    // uint8_t
    kVideoFullRangeFlag,
    // uint8_t
    kColourPrimaries,
    // uint8_t
    kTransferCharacteristics,
    // uint8_t
    kMatrixCoeffs,
    // keys required for MV-HEVC
    // uint8_t
    kMaxLayersMinusOne,
    // uint8_t
    kNumViews,
    // uint8_t
    kSpsMaxSubLayersMinusOne,
    // unit8_t
    kSeiLeftViewId,
};

class HevcParameterSets {
public:
    enum Info : uint32_t {
        kInfoNone                = 0,
        kInfoIsHdr               = 1 << 0,
        kInfoHasColorDescription = 1 << 1,
    };

    HevcParameterSets();
    HevcParameterSets(bool isMvHevc);

    status_t addNalUnit(const uint8_t* data, size_t size);

    bool findParam8(uint32_t key, uint8_t *param);
    bool findParam16(uint32_t key, uint16_t *param);
    bool findParam32(uint32_t key, uint32_t *param);
    bool findParam64(uint32_t key, uint64_t *param);

    inline size_t getNumNalUnits() { return mNalUnits.size(); }
    size_t getNumNalUnitsOfType(uint8_t type);
    // get number of Nalus using layerId as well
    size_t getNumNalUnitsOfType(uint8_t type, uint8_t layerId);
    // get payloadType of SEI message
    bool getThreeDimParamParsed();
    uint8_t getType(size_t index);
    size_t getSize(size_t index);
    // Note that this method does not write the start code.
    bool write(size_t index, uint8_t* dest, size_t size);
    status_t makeHvcc(uint8_t *hvcc, size_t *hvccSize, size_t nalSizeLength);
    status_t makeHvcc_l(uint8_t *hvcc, size_t *hvccSize, size_t nalSizeLength);
    void FindHEVCDimensions(
            const sp<ABuffer> &SpsBuffer, int32_t *width, int32_t *height);

    Info getInfo() const { return mInfo; }
    static bool IsHevcIDR(const uint8_t *data, size_t size);
    // for MV-HEVC
    bool IsMvHevc();
    void setMvHevc(bool isMvHevc) { mIsLhevc = isMvHevc; } // set mIsLhevc to true when mime type is video/x-mvhevc
    status_t makeLhvc(uint8_t *lhvc, size_t *lhvcSize, size_t nalSizeLength);
    void makeStri(uint8_t *stri);
    status_t makeHero(uint8_t *hero);
    size_t getLayerId(size_t index);

private:
    status_t parseVps(const uint8_t* data, size_t size);
    status_t parseSps(const uint8_t *data, size_t size, const uint8_t nuhLayerId);
    status_t parsePps(const uint8_t *data, size_t size, const uint8_t nuhLayerId);
    status_t parseProfileTierLevel(const bool profilePresentFlag, uint8_t maxNumSubLayersMinus1, NALBitReader &reader, const bool isInVps);
    status_t parseHrdParameters(const bool cprmsPresentFlag, uint8_t maxNumSubLayersMinus1, NALBitReader *reader);
    status_t parseSubLayerHrdParameters(const bool subPicHrdParamsPresentFlag, const uint8_t cpbCntMinus1, NALBitReader *reader);
    status_t parseVpsExtension(const uint8_t maxLayersMinus1, const bool baseLayerInternalFlag, NALBitReader &reader);
    status_t parseSeiMessage(const uint8_t *data, size_t size);
    status_t parseThreeDimensionalReferenceInfoSei(const uint8_t *data, size_t size);
    size_t numBitsParsedExpGolomb(uint8_t symbol);
    bool byteAligned(size_t bitCounter) { return bitCounter % 8 ? false : true; }

    KeyedVector<uint32_t, uint64_t> mParams;
    Vector<sp<ABuffer>> mNalUnits;

    // nal unit layer id vector
    Vector<uint8_t> mNalLayerIds;
    bool mIsLhevc;

    Info mInfo;

    DISALLOW_EVIL_CONSTRUCTORS(HevcParameterSets);
};

}  // namespace android

#endif  // HEVC_UTILS_H_
