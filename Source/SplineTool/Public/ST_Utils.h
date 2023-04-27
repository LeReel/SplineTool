#pragma once

DECLARE_LOG_CATEGORY_EXTERN(LogCustom, Log, All);

#define DEBUG_D(text, ...) UE_LOG(LogTemp, Display, TEXT(text), __VA_ARGS__);
#define DEBUG_W(text, ...) UE_LOG(LogTemp, Warning, TEXT(text), __VA_ARGS__);
#define DEBUG_E(text, ...) UE_LOG(LogTemp, Error, TEXT(text), __VA_ARGS__);
#define DEBUG_F(text, ...) UE_LOG(LogTemp, Fatal, TEXT(text), __VA_ARGS__);