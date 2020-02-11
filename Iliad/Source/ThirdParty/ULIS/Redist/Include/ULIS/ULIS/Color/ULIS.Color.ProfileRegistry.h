// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Color.ProfileRegistry.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

#include "ULIS/Color/ULIS.Color.ColorProfile.h"
#include <unordered_map>
#include <string>

namespace ULIS {
/////////////////////////////////////////////////////
// FProfileRegistry
class FProfileRegistry
{
public:
    // Construction / Destruction
    FProfileRegistry();
    ~FProfileRegistry();

public:
    // Public API
    FColorProfile* GetProfile( const std::string iKey );
    std::string DefaultProfileNameForModel( e_cm iModel );
    FColorProfile* GetDefaultProfileForModel( e_cm iModel );

private:
    // Private Data
    std::unordered_map< std::string, FColorProfile* > mLockedSoftwareProfiles;
    std::unordered_map< std::string, FColorProfile* > mFileBasedProfiles;
};


} // namespace ULIS
