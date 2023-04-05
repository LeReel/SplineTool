#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogCustom, Log, All);

#define DEBUG_D(owner, text) UE_LOG(LogCustom, Display, TEXT(text))
#define DEBUG_W(owner, text) UE_LOG(LogCustom, Warning, TEXT(text))
#define DEBUG_E(owner, text) UE_LOG(LogCustom, Error, TEXT(text))
#define DEBUG_F(owner, text) UE_LOG(LogCustom, Fatal, TEXT(text))
