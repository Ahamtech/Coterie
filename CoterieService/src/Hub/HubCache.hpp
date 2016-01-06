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

#ifndef UDSACCOUNT_HPP
#define UDSACCOUNT_HPP

#include <QSettings>

class HubCache: public QObject {

Q_OBJECT

Q_PROPERTY(int accountId READ accountId WRITE setAccountId)
Q_PROPERTY(QString accountName READ accountName WRITE setAccountName)

Q_PROPERTY(int lastCategoryId READ lastCategoryId)
Q_PROPERTY(QVariantList categories READ categories WRITE setCategories)

Q_PROPERTY(int lastItemId READ lastItemId)
Q_PROPERTY(QVariantList items READ items WRITE setItems)

public:
    /*
     * Constructor.
     *
     * @param settings - unique service name
     */
    HubCache(QSettings* settings);

    /*
     *  Destructor.
     */
    virtual ~HubCache();

    /*
     * Account ID of data in the cache.
     *
     * @returns qint64 account Id
     */
    qint64       accountId();

    /*
     * Account name corresponding to account ID of data in the cache.
     *
     * @returns QString name of account
     */
    QString      accountName();

    /*
     * Last category ID of data in the cache.
     *
     * @returns qint64 last category Id
     */
    qint64       lastCategoryId();

    /*
     * List of categories.
     *
     * @returns QVariantList current list of categories
     */
    QVariantList categories();

    /*
     * Last item ID of data in the cache
     *
     * @returns qint64 last item Id
     */
    qint64       lastItemId();

    /*
     * Get item by itemId from the cache.
     *
     * @param categoryId - category ID
     * @param itemId     - item ID
     *
     * @returns QVariant* pointer to item or NULL if no item matches the specified ID
     */
    QVariant*     getItem(qint64 categoryId, qint64 itemId);

    /*
     * Get item by syncId from the cache.
     *
     * @param categoryId - category ID
     * @param syncId     - sync ID
     *
     * @returns QVariant* pointer to item or NULL if no item matches the specified ID
     */
    QVariant*     getItemBySyncID(qint64 categoryId, QString syncId);
    QVariant getItemBySync(qint64 categoryId, QString syncId);

    /*
     * List of items in the cache.
     *
     * @returns QVariantList list of items.
     */
    QVariantList items();

public Q_SLOTS:

    /*
     * Set account ID for the current account in the cache.
     *
     * @param accountId - account ID
     */
    void setAccountId(int accountId);

    /*
     * Set name of account in the cache.
     *
     * @param accountName - account name
     */
    void setAccountName(QString accountName);

    /*
     * Replace list of categories in the cache with the provided list.
     *
     * @param categories - new list of categories
     */
    void setCategories(QVariantList categories);

    /*
     * Add a new item in the cache using the provided data.
     *
     * @param itemMap - item data
     */
    void addItem(QVariantMap itemMap);

    /*
     * Updated the specified item in the cache with the provided data.
     *
     * @param itemId - ID of the item to be deleted
     * @param itemMap - item data
     */
    void updateItem(qint64 itemId, QVariantMap itemMap);

    /*
     * Remove the specified item from the cache.
     *
     * @param itemId - ID of the item to be deleted
     */
    void removeItem(qint64 itemId);

    /*
     * Replace list of items in the cache with the list provided.
     *
     * @param items - new list of items
     */
    void setItems(QVariantList items);

private:
    QString      _accountIdKey;
    QString      _accountNameKey;

    QString      _categoryIdKey;
    QString      _categoriesKey;
    QString      _categoryNameKey;

    QString      _lastItemIdKey;
    QString      _itemsKey;


    int          _accountId;
    QString      _accountName;

    QVariantList _categories;

    int          _lastItemId;
    QVariantList _items;

    QSettings*   _settings;
};

#endif /* UDSACCOUNT_HPP */
