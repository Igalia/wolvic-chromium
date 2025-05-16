// Copyright 2011 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef COMPONENTS_EXTENSIONS_BROWSER_API_BOOKMARKS_BOOKMARK_API_CONSTANTS_H_
#define COMPONENTS_EXTENSIONS_BROWSER_API_BOOKMARKS_BOOKMARK_API_CONSTANTS_H_

// Constants used for the Bookmarks API.

namespace extensions {
namespace bookmark_api_constants {

// Keys.
extern const char kParentIdKey[];
extern const char kUrlKey[];
extern const char kTitleKey[];

// Errors.
extern const char kNoNodeError[];
extern const char kNoParentError[];
extern const char kFolderNotEmptyError[];
extern const char kInvalidIdError[];
extern const char kInvalidIndexError[];
extern const char kInvalidParentError[];
extern const char kInvalidUrlError[];
extern const char kModifySpecialError[];
extern const char kEditBookmarksDisabled[];
extern const char kModifyManagedError[];
extern const char kInvalidParamError[];
extern const char kCannotSetUrlOfFolderError[];
extern const char kBookmarkNodesNotFoundFromIdListError[];

}  // namespace bookmark_api_constants
}  // namespace extensions

#endif  // COMPONENTS_EXTENSIONS_BROWSER_API_BOOKMARKS_BOOKMARK_API_CONSTANTS_H_
