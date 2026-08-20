// Copyright Picardbuilds. All Rights Reserved.

#include "HasardSaveGame.h"

// Named for the game rather than something generic: slots live in a shared folder per
// user, so "Session" alone would be a collision waiting for the second project.
const TCHAR* UHasardSaveGame::SlotName = TEXT("HasardSession");