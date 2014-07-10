/*
 Copyright (c) 2011-2012, salesforce.com, inc. All rights reserved.
 
 Redistribution and use of this software in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:
 * Redistributions of source code must retain the above copyright notice, this list of conditions
 and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright notice, this list of
 conditions and the following disclaimer in the documentation and/or other materials provided
 with the distribution.
 * Neither the name of salesforce.com, inc. nor the names of its contributors may be used to
 endorse or promote products derived from this software without specific prior written
 permission of salesforce.com, inc.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
 IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY
 WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */


#import <Foundation/Foundation.h>


/**
 The default store name used by the SFSmartStorePlugin: native code may choose
 to use separate stores.
 */
extern NSString *const kDefaultSmartStoreName;

/**
 The NSError domain for SmartStore errors.
 */
extern NSString * const kSFSmartStoreErrorDomain;

/**
 The label used to interact with the encryption key.
 */
extern NSString * const kSFSmartStoreEncryptionKeyLabel;

/**
 Block typedef for generating an encryption key.
 */
typedef NSString* (^SFSmartStoreEncryptionKeyBlock)(void);

@class FMDatabaseQueue;
@class SFStoreCursor;
@class SFQuerySpec;
@class SFUserAccount;

@interface SFSmartStore : NSObject {

    //used for monitoring the status of file data protection
    BOOL    _dataProtectionKnownAvailable;
    id      _dataProtectAvailObserverToken;
    id      _dataProtectUnavailObserverToken;
    
    FMDatabaseQueue *_storeQueue;
    NSString *_storeName;
    
    NSMutableDictionary *_indexSpecsBySoup;
    NSMutableDictionary *_smartSqlToSql;
}

/**
 The name of this store. 
 */
@property (nonatomic, readonly, strong) NSString *storeName;


/**
 Use this method to obtain a shared store instance with a particular name for the current user.
 
 @param storeName The name of the store.  If in doubt, use kDefaultSmartStoreName.
 @return A shared instance of a store with the given name.
 */
+ (id)sharedStoreWithName:(NSString*)storeName;

/**
 Use this method to obtain a shared store instance with the given name for the given user.
 @param storeName The name of the store.  If in doubt, use kDefaultSmartStoreName.
 @param user The user associated with the store.
 */
+ (id)sharedStoreWithName:(NSString*)storeName user:(SFUserAccount *)user;

/**
 You may use this method to completely remove a persistent shared store with
 the given name for the current user.
 
 @param storeName The name of the store. 
 */
+ (void)removeSharedStoreWithName:(NSString *)storeName;

/**
 You may use this method to completely remove a persisted shared store with the given name
 for the given user.
 @param storeName The name of the store to remove.
 @param user The user associated with the store.
 */
+ (void)removeSharedStoreWithName:(NSString *)storeName forUser:(SFUserAccount *)user;

/**
 Removes all of the stores for the current user from this app.
 */
+ (void)removeAllStores;

/**
 Removes all of the store for the given user from this app.
 @param user The user associated with the stores to remove.
 */
+ (void)removeAllStoresForUser:(SFUserAccount *)user;

/**
 @return The block used to generate the encryption key.  Sticking with the default encryption
 key derivation is recommended.
 */
+ (SFSmartStoreEncryptionKeyBlock)encryptionKeyBlock;

/**
 Sets a custom block for deriving the encryption key used to encrypt stores.
 
 ** WARNING: **
 If you choose to override the encryption key derivation, you must set
 this value before opening any stores.  Setting the value after stores have been opened
 will result in the corruption and loss of existing data.
 ** WARNING: **
 
 @param newEncryptionKeyBlock The new encryption key derivation block to use with SmartStore.
 */
+ (void)setEncryptionKeyBlock:(SFSmartStoreEncryptionKeyBlock)newEncryptionKeyBlock;

#pragma mark - Soup manipulation methods

/**
 @param soupName the name of the soup
 @return NSArray of SFSoupIndex for the given soup
 */
- (NSArray*)indicesForSoup:(NSString*)soupName;

/**
 @param soupName the name of the soup
 @return Does a soup with the given name already exist?
 */
- (BOOL)soupExists:(NSString*)soupName;

/**
 Ensure that a soup with the given name exists.
 Either creates a new soup or returns an existing soup.
 
 @param soupName The name of the soup to register
 @param indexSpecs Array of one ore more IndexSpec objects as dictionaries
 @return YES if the soup registered OK
 */
- (BOOL)registerSoup:(NSString*)soupName withIndexSpecs:(NSArray*)indexSpecs;


/**
 Get the number of entries that would be returned with the given query spec
 
 @param querySpec a native query spec
 */
- (NSUInteger)countWithQuerySpec:(SFQuerySpec*)querySpec;

/**
 Search for entries matching the querySpec

 @param querySpec A querySpec as a dictionary
 @param targetSoupName the soup name targeted (not nil for exact/like/range queries)

 @return A cursor
 */
- (SFStoreCursor*)queryWithQuerySpec:(NSDictionary *)querySpec withSoupName:(NSString*) targetSoupName;


/**
 Search for entries matching the querySpec
 
 @param querySpec A native SFSoupQuerySpec
 @param pageIndex The page index to start the entries at (this supports paging)
 @return A set of entries given the pageSize provided in the querySpec
 */
- (NSArray *)queryWithQuerySpec:(SFQuerySpec *)querySpec pageIndex:(NSUInteger)pageIndex;

/**
 Search soup for entries exactly matching the soup entry IDs
 
 @param soupName The name of the soup to query
 @param soupEntryIds An array of opaque soup entry IDs
 
 @return An array with zero or more entries matching the input IDs. Order is not guaranteed.
 */
- (NSArray*)retrieveEntries:(NSArray*)soupEntryIds fromSoup:(NSString*)soupName;

/**
 Insert/update entries to the soup.  Insert vs. update will be determined by the internal
 soup entry ID generated from intial entry.  If you want to specify a different identifier
 for determining existing entries, use upsertEntries:toSoup:withExternalIdPath:
 
 @param entries The entries to insert or update.
 @param soupName The name of the soup to update.
 
 @return The array of updated entries in the soup.
 */
- (NSArray*)upsertEntries:(NSArray*)entries toSoup:(NSString*)soupName;

/**
 Insert/update entries to the soup.  Insert vs. update will be determined by the specified
 external ID path argument.
 
 @param entries The entries to insert or update.
 @param soupName The name of the soup to update.
 @param externalIdPath The user-defined query spec path used to determine insert vs. update.
 @param error Sets/returns any error generated as part of the process.
 
 @return The array of updated entries in the soup.
 */
- (NSArray *)upsertEntries:(NSArray *)entries toSoup:(NSString *)soupName withExternalIdPath:(NSString *)externalIdPath error:(NSError **)error;

/**
 Remove soup entries exactly matching the soup entry IDs
 
 @param entryIds An array of opaque soup entry IDs from _soupEntryId
 @param soupName The name of the soup from which to remove the soup entries
 
 */
- (void)removeEntries:(NSArray*)entryIds fromSoup:(NSString*)soupName;


/**
 Remove soup completely from the store.
 
 @param soupName The name of the soup to remove from the store.
 */
- (void)removeSoup:(NSString*)soupName;

/**
 Remove all soups from the store.
 */
- (void)removeAllSoups;


#pragma mark - Utility methods

/**
 This is updated based on receiving notifications for
 UIApplicationProtectedDataDidBecomeAvailable / UIApplicationProtectedDataWillBecomeUnavailable.
 Note that on the simulator currently, data protection is NEVER active.
 
 @return Are we sure that file data protection (full passcode-based encryption) is available?
 */
- (BOOL)isFileDataProtectionActive;


@end
