//
// Copyright (c) Microsoft Corporation.  All rights reserved.
//
//
// Use of this source code is subject to the terms of the Microsoft
// premium shared source license agreement under which you licensed
// this source code. If you did not accept the terms of the license
// agreement, you are not authorized to use this source code.
// For the terms of the license, please see the license agreement
// signed by you and Microsoft.
// THE SOURCE CODE IS PROVIDED "AS IS", WITH NO WARRANTIES OR INDEMNITIES.
//
// ============================================================================
// Library STLExtensionUty
//      A repository for useful extensions to STL algorithms and classes
//
// History:
//      01/24/2000 ｷ ericflee   ｷ created empty library
//      01/26/2000 ｷ ericflee   ｷ added for_all algorithm
// ============================================================================


#ifndef __STLEXTENSIONUTY_H__
#define __STLEXTENSIONUTY_H__

// external dependencies
// 覧覧覧覧覧覧覧覧覧覧�
#include <algorithm>

namespace STLExtensionUty
{

    // algorithm for_all
    // 覧覧覧覧覧覧覧覧�
    //     evalutes op( *itr ) once, for each element in the given container
    template <class TContainer, class TOperator>
    TOperator for_all(TContainer& container, TOperator op)
    {
        return std::for_each(container.begin(), container.end(), op);
    }

}

#endif