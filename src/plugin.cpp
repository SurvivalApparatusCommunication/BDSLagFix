/**
 * @file plugin.cpp
 * @brief The main file of the plugin
 */

// #include <llapi/LoggerAPI.h>
#include <llapi/mc/BlockTypeRegistry.hpp>
#include <llapi/mc/Block.hpp>
#include <llapi/mc/HashedString.hpp>
#include <llapi/mc/Experiments.hpp>
#include <llapi/mc/BlockLegacy.hpp>
#include <llapi/mc/BlockPalette.hpp>
#include <parallel_hashmap/phmap.h>

#include "version.h"

// We recommend using the global logger.
// // extern Logger logger;

/**
 * @brief The entrypoint of the plugin. DO NOT remove or rename this function.
 *
 */
void PluginInit() {}
phmap::flat_hash_map<uint64_t, Block const*> blockMap;
phmap::flat_hash_map<std::string, BlockLegacy*> blockLegacyMap;
phmap::flat_hash_map<uint64_t, BlockLegacy*> blockLegacyHashedMap;

THook(void, "?assignBlocks@BedrockBlocks@@YAXXZ") {
    for (auto& sb : BlockTypeRegistry::getBlockLookupMap()) {
        if (!blockMap.contains(sb.first.hash)) {
            blockMap[sb.first.hash] = &sb.second->getRenderBlock();
        }
        if (!blockLegacyMap.contains(sb.first.str)) {
            blockLegacyMap[sb.first.str] = sb.second.get();
        }
        if (!blockLegacyHashedMap.contains(sb.first.hash)) {
            blockLegacyHashedMap[sb.first.hash] = sb.second.get();
        }
    }
    original();
}
// "?mBlockLookupMap@BlockTypeRegistry@@0V?$map@VHashedString@@V?$SharedPtr@VBlockLegacy@@@@U?$less@VHashedString@@@std@@V?$allocator@U?$pair@$$CBVHashedString@@V?$SharedPtr@VBlockLegacy@@@@@std@@@4@@std@@A
// dq ?"
// "?mBlockLookupMap@BlockTypeRegistry@@2V?$map@VHashedString@@V?$SharedPtr@VBlockLegacy@@@@U?$less@VHashedString@@@std@@V?$allocator@U?$pair@$$CBVHashedString@@V?"

THook(Block const&,
      "?getDefaultBlockState@BlockTypeRegistry@@SAAEBVBlock@@AEBVHashedString@@_N@Z",
      class HashedString const& name,
      bool sb) {
    auto p = blockMap.find(name.hash);
    if (p == blockMap.end()) {
        // logger.warn("getDefaultBlockState {} {}", name.str, sb);
        return original(name, sb);
    }
    return *((*p).second);
}
THook(Block const*,
      "?lookupByName@BlockTypeRegistry@@SAPEBVBlock@@AEBVHashedString@@H_N@Z",
      class HashedString const& name,
      int sb2,
      bool sb) {
    auto p = blockMap.find(name.hash);
    if (p == blockMap.end()) {
        // logger.warn("lookupByName {} {}", name.str, sb);
        return original(name, sb2, sb);
    }
    return (*p).second;
}
THook(WeakPtr<class BlockLegacy>,
      "?lookupByName@BlockTypeRegistry@@SA?AV?$WeakPtr@VBlockLegacy@@@@AEBVHashedString@@_N@Z",
      class HashedString const& name,
      bool sb) {
    auto p = blockLegacyHashedMap.find(name.hash);
    if (p == blockLegacyHashedMap.end()) {
        // logger.warn("lookupByName {} {}", name.str, sb);
        return original(name, sb);
    }
    return (*p).second;
}
THook(void, "?unregisterBlock@BlockTypeRegistry@@SAXAEBVHashedString@@@Z", class HashedString const& name) {
    original(name);
    blockMap.erase(name.hash);
}
THook(void, "?unregisterBlocks@BlockTypeRegistry@@SAXXZ") {
    original();
    blockMap.clear();
}

TInstanceHook(class BlockLegacy const*,
              "?getBlockLegacy@BlockPalette@@QEBAPEBVBlockLegacy@@AEBV?$basic_string@DU?$char_"
              "traits@D@std@@V?$allocator@D@2@@std@@@Z",
              BlockPalette,
              std::string const& name) {
    auto p = blockLegacyMap.find(name);
    if (p == blockLegacyMap.end()) {
        // logger.warn("getBlockLegacy {}", name);
        return original(this, name);
    }
    return (*p).second;
}
// "?mBlockLookupMap@BlockTypeRegistry@@0V?$map@VHashedString@@V?$SharedPtr@VBlockLegacy@@@@U?$less@VHashedString@@@std@@V?$allocator@U?$pair@$$CBVHashedString@@V?$SharedPtr@VBlockLegacy@@@@@std@@@4@@std@@A"
// "?mBlockLookupMap@BlockTypeRegistry@@2V?$map@VHashedString@@V?$SharedPtr@VBlockLegacy@@@@U?$less@VHashedString@@@std@@V?$allocator@U?$pair@$$CBVHashedString@@V?"