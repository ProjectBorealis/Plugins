// Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
// IDDN FR.001.250001.002.S.P.2019.000.00000

/**
 * @file        ULIS.Core.h
 * @author      Clement Berthaud
 * @copyright   Copyright © 2018-2019 Praxinos, Inc. All Rights Reserved.
 * @license     Please refer to LICENSE.md
 */

#pragma once

// Warning ! The include order does matter !
#include "ULIS/ULIS.Config.h"
#include "ULIS/Interface/ULIS.Interface.Decl.h"
#include "ULIS/Interface/ULIS.Interface.Access.h"
#include "ULIS/Interface/ULIS.Interface.Make.h"
#include "ULIS/Interface/ULIS.Interface.Spec.h"
#include "ULIS/Interface/ULIS.Interface.Blend.h"
#include "ULIS/Interface/ULIS.Interface.ClearFill.h"
#include "ULIS/Interface/ULIS.Interface.Conv.h"
#include "ULIS/Interface/ULIS.Interface.Paint.h"
#include "ULIS/Interface/ULIS.Interface.Transform.h"
#include "ULIS/Interface/ULIS.Interface.FX.h"
#include "ULIS/Global/ULIS.Global.GlobalThreadPool.h"
#include "ULIS/Global/ULIS.Global.GlobalCPUConfig.h"
#include "ULIS/Global/ULIS.Global.GlobalPalettes.h"
#include "ULIS/Global/ULIS.Global.GlobalProfileRegistry.h"
#include "ULIS/Thread/ULIS.Thread.ParallelFor.h"

