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

#include <math.h>

#include "HubCache.hpp"

#include <QDebug>

HubCache::HubCache(QSettings* settings) :
                    _accountIdKey("hub/accountId"),
                    _accountNameKey("hub/accountName"),
                    _categoryIdKey("hub/categoryId"),
                    _categoriesKey("hub/categories"),
                    _categoryNameKey("hub/categoryName"),
                    _lastItemIdKey("hub/lastItemId"),
                    _itemsKey("hub/items"),
                    _settings(settings)
{
    qDebug()  << "HubCache::HubCache ";

    _accountId = -1;
    _lastItemId = 0;
}

HubCache::~HubCache() {
    // TODO Auto-generated destructor stub
}

qint64 HubCache::accountId()
{
    if (_settings->contains(_accountIdKey)) {
        _accountId = _settings->value(_accountIdKey).toLongLong();
    }

    return _accountId;
}

QString HubCache::accountName()
{
    if (_settings->contains(_accountNameKey)) {
        _accountName = _settings->value(_accountNameKey).toString();
    }

    return _accountName;
}

qint64 HubCache::lastCategoryId()
{
    return _categories.size();
}

QVariantList HubCache::categories()
{
    if (_settings->contains(_categoryIdKey)) {
        QVariantMap category;
        category["categoryId"] = _settings->value(_categoryIdKey).toLongLong();
        category["name"] = _settings->value(_categoryNameKey).toString();
        category["parentCategoryId"] = 0; // default parent category ID for root categories

        _categories << category;

        // legacy QSettings cache restore - should only be called once if updating from older code
        _settings->remove(_categoryIdKey);
        _settings->remove(_categoryNameKey);

        _settings->setValue(_categoriesKey, _categories);

    } else if (_settings->contains(_categoriesKey)) {
        _categories = _settings->value(_categoriesKey).toList();
    }

    return _categories;
}

qint64 HubCache::lastItemId()
{
    if (_settings->contains(_lastItemIdKey)) {
        _lastItemId = _settings->value(_lastItemIdKey).toInt();
    }

    return _lastItemId;
}

QVariant* HubCache::getItem(qint64 categoryId, qint64 itemId)
{
    QVariant* foundItem = NULL;

    for(int index = 0; index < _items.size(); index++) {
        QVariantMap item = _items.at(index).toMap();

//        qDebug()  << "HubCache::getItem " << index << ":" << item;

        if (item["categoryId"].toLongLong() == categoryId && item["sourceId"].toLongLong() == itemId) {
            foundItem = new QVariant(item);
            break;
        }
    }

    return foundItem;
}

QVariant* HubCache::getItemBySyncID(qint64 categoryId, QString syncId)
{
    QVariant* foundItem = NULL;

    for(int index = 0; index < _items.size(); index++) {
        QVariantMap item = _items.at(index).toMap();
        if (item["categoryId"].toLongLong() == categoryId && item["syncId"].toString() == syncId) {
            foundItem = new QVariant(item);
            break;
        }
    }

    return foundItem;
}

QVariant HubCache::getItemBySync(qint64 categoryId, QString syncId)
{
    QVariant foundItem = NULL;

    for(int index = 0; index < _items.size(); index++) {
        QVariantMap item = _items.at(index).toMap();
        if (item["categoryId"].toLongLong() == categoryId && item["syncId"].toString() == syncId) {
            foundItem = item;
            break;
        }
    }

    return foundItem;
}

QVariantList HubCache::items()
{
    if (_settings->contains(_itemsKey) && _items.size() == 0) {
        _items = _settings->value(_itemsKey).toList();
    }

    return _items;
}

void HubCache::setAccountId(int accountId)
{
    _accountId = accountId;

    if (_accountId > 0) {
        _settings->setValue(_accountIdKey, _accountId);
    }
}

void HubCache::setAccountName(QString accountName)
{
    _accountName = accountName;

    if (_accountName.length() > 0) {
        _settings->setValue(_accountNameKey, _accountName);
    }
}

void HubCache::setCategories(QVariantList categories)
{
    _categories = categories;

    _settings->setValue(_categoriesKey, _categories);
}

void HubCache::addItem(QVariantMap itemMap)
{
    _items.append(itemMap);
    _lastItemId++;

    _settings->setValue(_lastItemIdKey, _lastItemId);
    _settings->setValue(_itemsKey, _items);
}

void HubCache::updateItem(qint64 itemId, QVariantMap itemMap)
{
    if (itemId > 0) {
        for(int index = 0; index < _items.size(); index++) {
            QVariantMap item = _items.at(index).toMap();
            if (item["sourceId"].toLongLong() == itemId) {
                _items[index] = itemMap;
                break;
            }
        }
    }

    _settings->setValue(_itemsKey, _items);
}

void HubCache::removeItem(qint64 itemId)
{
    if (itemId > 0) {
        for(int index = 0; index < _items.size(); index++) {
            QVariantMap item = _items.at(index).toMap();
            if (item["sourceId"].toLongLong() == itemId) {
                _items.removeAt(index);
                if (itemId == _lastItemId) {
                    _lastItemId--;
                }
                break;
            }
        }
    }

    _settings->setValue(_lastItemIdKey, _lastItemId);
    _settings->setValue(_itemsKey, _items);
}

void HubCache::setItems(QVariantList items)
{
    _items = items;

    _settings->setValue(_itemsKey, _items);
}
