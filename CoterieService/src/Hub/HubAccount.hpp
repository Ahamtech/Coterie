/*
 * Copyright (c) 2013 BlackBerry Limited.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef HUBACCOUNT_HPP
#define HUBACCOUNT_HPP

#include <QStringList>

#include "UDSUtil.hpp"
#include "HubCache.hpp"

class HubAccount: public QObject {

Q_OBJECT

public:
    /*
     * Constructor.
     *
     * @param udsUtil - UDS utility class for use by this account
     * @param hubCache - hub cache
     */
    HubAccount(UDSUtil* udsUtil, HubCache* hubCache);

    /*
     *  Destructor.
     */
    virtual ~HubAccount();

    /*
     *  Initialize the Hub account.
     */
    virtual void initialize();

    /*
     * Initialize the categories for this Hub account.
     *
     * @param categories - list of category names to create
     */
    virtual void initializeCategories(QVariantList categories);
    void updateAccount(QString,QString);

    /*
     *  Remove the Hub account.
     */
    bool remove();

public Q_SLOTS:
    /*
     * Returns the categories for this Hub account.
     *
     * @returns QVariantMap - categories for this account
     */
    QVariantList categories();

    /*
     * Get Hub item by itemId from the cache.
     *
     * @param categoryId - category ID
     * @param itemId     - item ID
     *
     * @returns QVariant* pointer to item or NULL if no item matches the specified ID
     */
    QVariant* getHubItem(qint64 categoryId, qint64 itemId);

    /*
     * Get Hub item by syncId from the cache.
     *
     * @param categoryId - category ID
     * @param syncId     - sync ID
     *
     * @returns QVariant* pointer to item or NULL if no item matches the specified ID
     */
    QVariant* getHubItemBySyncID(qint64 categoryId, QString syncId);
    QVariant getHubItemBySync(qint64 categoryId, QString syncId);
    /*
     * List of items in the cache.
     *
     * @returns QVariantList list of items.
     */
    QVariantList items();

    /*
     * Add a new category for the Hub account using the provided data.
     *
     * @param parentCategoryId - category ID
     * @param name             - category name
     */
    bool addHubCategory(qint64 parentCategoryId, QString name);

    /*
     * Update category for the Hub account using the provided data.
     *
     * @param categoryId       - category ID
     * @param parentCategoryId - category ID
     * @param name             - category name
     */
    bool updateHubCategory(qint64 categoryId, qint64 parentCategoryId, QString name);

    /*
     * Remove category from the Hub account.
     *
     * @param categoryId       - category ID
     */
    bool removeHubCategory(qint64 categoryId);

    /*
     * Add a new item for the Hub account using the provided data.
     *
     * @param categoryId - category ID
     * @param itemMap - item data
     * @param name - item name
     * @param subject - item subject
     * @param timestamp - item timestamp
     * @param itemSyncId - item syncId
     * @param itemUserData     - extra data specific to the app that is associated with this item
     * @param itemExtendedData - extended data that controls special properties of Hub items
     * @param notify - true if a notification is to be generated, false otherwise
     */
    bool addHubItem(qint64 categoryId, QVariantMap &itemMap, QString name, QString subject, qint64 timestamp, QString itemSyncId, QString itemUserData, QString itemExtendedData, bool notify);

    /*
     * Updated the specified item in the Hub account / cache with the provided data.
     *
     * @param categoryId - category ID
     * @param itemId - ID of the item to be deleted
     * @param itemMap - item data
     * @param notify - true if a notification is to be generated, false otherwise
     */
    bool updateHubItem(qint64 categoryId, qint64 itemId, QVariantMap &itemMap, bool notify);

    /*
     * Remove the specified item from the Hub account / cache.
     *
     * @param categoryId - category ID
     * @param itemId - ID of the item to be deleted
     */
    bool removeHubItem(qint64 categoryId, qint64 itemId);

    // convenience functions for common Hub operations
    /*
     * Mark the specified item from the Hub account / cache as read.
     *
     * @param categoryId - category ID
     * @param itemId - ID of the item to be marked as read
     */
    bool markHubItemRead(qint64 categoryId, qint64 itemId);

    /*
     * Mark the specified item from the Hub account / cache as unread.
     *
     * @param categoryId - category ID
     * @param itemId - ID of the item to be marked as unread
     */
    bool markHubItemUnread(qint64 categoryId, qint64 itemId);

    /*
     * Mark the items from the Hub account / cache, older than the timestamp provided, as read.
     *
     * @param categoryId - category ID
     * @param timestamp - timestamp before items are to be marked as read
     */
    void markHubItemsReadBefore(qint64 categoryId, qint64 timestamp);

    /*
     * Delete the items from the Hub account / cache that are older than the timestamp provided..
     *
     * @param categoryId - category ID
     * @param timestamp - timestamp before items are to be marked as read
     */
    void removeHubItemsBefore(qint64 categoryId, qint64 timestamp);

    /*
     * Clear hub of all items and categories in the cache.
     *
     */
    void clearHub();

    /*
     * Repopulate Hub with categories and items from the cache.
     *
     */
    void repopulateHub();

protected:
    UDSUtil*  _udsUtil;
    HubCache* _hubCache;

    bool    _initialized;
    bool    _categoriesInitialized;

    qint64 _accountId;

    QString _name;
    QString _displayName;
    QString _description;

    QString _serverName;

    QString _iconFilename;
    QString _lockedIconFilename;
    QString _composeIconFilename;

    bool    _supportsCompose;
    bool    _supportsMarkRead;
    bool    _supportsMarkUnread;

    QString _headlessTarget;
    QString _appTarget;
    QString _cardTarget;

    QString _itemMimeType;
    QString _itemComposeIconFilename;
    QString _itemReadIconFilename;
    QString _itemUnreadIconFilename;

    QString _markReadActionIconFilename;
    QString _markUnreadActionIconFilename;
};

#endif /* HUBACCOUNT_HPP */
