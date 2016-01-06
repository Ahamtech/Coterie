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

#include "HubAccount.hpp"

#include <QDebug>
#include <QVariantList>

HubAccount::HubAccount(UDSUtil* udsUtil, HubCache* hubCache) :
        _udsUtil(udsUtil), _hubCache(hubCache)
{
    qDebug() << "HubAccount::HubAccount " << udsUtil;

    _accountId = 0;
    _supportsCompose = false;
    _supportsMarkRead = false;
    _supportsMarkUnread = false;
    _initialized = false;
    _categoriesInitialized = false;
}

HubAccount::~HubAccount()
{
    // TODO Auto-generated destructor stub
}

QVariantList HubAccount::categories()
{
    return _hubCache->categories();
}

void HubAccount::initialize()
{
    qDebug() << "HubAccount::initialize " << _initialized;

    if (!_initialized) {
        int retVal = -1;

        _accountId = _hubCache->accountId();

        if (_hubCache->accountId() < 0) {
            _accountId = _udsUtil->addAccount(_name, _displayName, _serverName, _headlessTarget,
                    _iconFilename, _lockedIconFilename, _composeIconFilename, _description,
                    _supportsCompose, UDS_ACCOUNT_TYPE_OTHER);

            if (_accountId > 0) {
                qDebug() << "HubAccount::initialize: addAccount succeeded for account "
                        << _accountId << " name: " << _name << "\n";

                _hubCache->setAccountId(_accountId);
                _hubCache->setAccountName(_name);
            } else {
                qDebug() << "HubAccount::initialize: addAccount failed for account name: " << _name
                        << "\n";
            }

            if (_supportsCompose) {
                int retVal = _udsUtil->addAccountAction(_accountId, QString("bb.action.COMPOSE"),
                        QString(tr("Compose")), _cardTarget, QString("application"),
                        _itemComposeIconFilename, _itemMimeType, UDS_PLACEMENT_BAR);
                if (retVal != 0) {
                    qDebug() << "HubAccount::initialize: addAccountActionData: bb.action.COMPOSE : "
                            << " retval: " << retVal;
                }
            }

            if (_supportsMarkRead) {
                retVal = _udsUtil->addItemAction(_accountId, QString("bb.action.MARKREAD"),
                        QString("Mark Read"), _headlessTarget, QString("application.headless"),
                        _markReadActionIconFilename, _itemMimeType, UDS_PLACEMENT_OVERFLOW);
                if (retVal != 0) {
                    qDebug()
                            << "HubAccount::addHubItem: addItmActionData: addItmAction: bb.action.MARKREAD : "
                            << " retval: " << retVal;
                }
            }

            if (_supportsMarkUnread) {
                retVal = _udsUtil->addItemAction(_accountId, QString("bb.action.MARKUNREAD"),
                        QString("Mark Unread"), _headlessTarget,
                        QString("application.headless"), _markUnreadActionIconFilename,
                        _itemMimeType, UDS_PLACEMENT_OVERFLOW);
                if (retVal != 0) {
                    qDebug()
                            << "HubAccount::addHubItem: addItmActionData: addItmAction: bb.action.MARKUNREAD : "
                            << " retval: " << retVal;
                }
            }
        } else {
            QString accountName = _hubCache->accountName();

            _udsUtil->restoreNextIds(_accountId + 1, _hubCache->lastCategoryId() + 1,
                    _hubCache->lastItemId() + 1);
        }

        _initialized = true;
    }
}

void HubAccount::initializeCategories(QVariantList newCategories)
{
    qDebug() << "HubAccount::initializeCategories " << _categoriesInitialized;

    if (!_categoriesInitialized) {
        qint64 retVal = -1;

        if (_hubCache->categories().size() == 0) {
            QVariantList categories;

            for (int index = 0; index < newCategories.size(); index++) {
                QVariantMap category = newCategories[index].toMap();
                retVal = _udsUtil->addCategory(_accountId, category["name"].toString(),
                        category["parentCategoryId"].toLongLong());
                if (retVal == -1) {
                    qDebug() << "HubAccount::initializeCategories: add category failed for: "
                            << category;
                    break;
                }

                category["categoryId"] = retVal;
                categories << category;
            }

            if (retVal > 0) {
                _hubCache->setCategories(categories);
            }
        }

        QVariantList items = _hubCache->items();

        _categoriesInitialized = true;
    }
}

bool HubAccount::remove()
{

    return _udsUtil->removeAccount(_accountId);

    _accountId = 0;
}

QVariant* HubAccount::getHubItem(qint64 categoryId, qint64 itemId)
{
    QVariant* item = _hubCache->getItem(categoryId, itemId);
    if (item) {
        QVariantMap itemMap = (*item).toMap();
    }

    return item;
}

QVariant* HubAccount::getHubItemBySyncID(qint64 categoryId, QString syncId)
{
    QVariant* item = _hubCache->getItemBySyncID(categoryId, syncId);
    if (item) {
        QVariantMap itemMap = (*item).toMap();
    }

    return item;
}

QVariant HubAccount::getHubItemBySync(qint64 categoryId, QString syncId)
{
    QVariant items = NULL;
    QVariantList _items = _hubCache->items();
    if (_items.size() > 0) {
        for (int index = 0; index < _items.size(); index++) {
            QVariantMap item = _items.at(index).toMap();
            if (item["categoryId"].toLongLong() == categoryId && item["syncId"].toString() == syncId) {
                items = item;
                break;
            }
        }
    }

    return items;
}

QVariantList HubAccount::items()
{
    QVariantList _items = _hubCache->items();
    QVariantList items;

    if (_items.size() > 0) {
        for (int index = 0; index < _items.size(); index++) {
            QVariantMap itemMap = _items.at(index).toMap();

            items << itemMap;
        }
    }

    return items;
}

bool HubAccount::addHubCategory(qint64 parentCategoryId, QString name)
{
    qint64 retVal = 0;

    qDebug() << "add hub category: " << parentCategoryId << " - " << name;

    QVariantMap category;
    category["accountId"] = _accountId;
    category["name"] = name;
    category["parentCategoryId"] = parentCategoryId;

    retVal = _udsUtil->addCategory(_accountId, name, parentCategoryId);

    if (retVal <= 0) {
        qDebug() << "HubAccount::addHubCategory: addCategory failed for category: " << name
                << ", account: " << _accountId << ", retVal: " << retVal << "\n";
    } else {
        QVariantList categories = this->categories();

        category["categoryId"] = retVal;
        categories << category;

        _hubCache->setCategories(categories);
    }

    return (retVal > 0);
}

bool HubAccount::updateHubCategory(qint64 categoryId, qint64 parentCategoryId, QString name)
{
    qint64 retVal = 0;

    qDebug() << "update hub category: " << categoryId << " - " << parentCategoryId << " - " << name;

    QVariantList categories = this->categories();

    for (int index = 0; index < categories.size(); index++) {
        QVariant category = categories.at(index);
        QVariantMap categoryMap = category.toMap();
        qint64 cacheCategoryId = categoryMap["categoryId"].toLongLong();

        if (categoryId == cacheCategoryId) {
            categoryMap["name"] = name;
            categoryMap["parentCategoryId"] = parentCategoryId;

            retVal = _udsUtil->updateCategory(_accountId, categoryId, name, parentCategoryId);

            if (retVal <= 0) {
                qDebug() << "HubAccount::updateHubCategory: updateCategory failed for category: "
                        << name << ", account: " << _accountId << ", retVal: " << retVal << "\n";
            } else {
                _hubCache->setCategories(categories);
            }
            break;
        }
    }

    return (retVal > 0);
}

bool HubAccount::removeHubCategory(qint64 categoryId)
{
    bool retval = false;

    qDebug() << "remove hub category: " << categoryId;

    QVariantList categories = this->categories();
    QVariantList newCategories;

    for (int index = 0; index < categories.size(); index++) {
        QVariant category = categories.at(index);
        QVariantMap categoryMap = category.toMap();
        qint64 cacheCategoryId = categoryMap["categoryId"].toLongLong();

        if (categoryId == cacheCategoryId) {
            retval = _udsUtil->removeCategory(_accountId, categoryId);

            if (!retval) {
                qDebug() << "HubAccount::removeHubCategory: removeCategory failed for category: "
                        << categoryId << ", account: " << _accountId << ", retval: " << retval
                        << "\n";
                break;
            }
        } else {
            newCategories << category;
        }
    }

    if (retval) {
        _hubCache->setCategories(newCategories);
    }

    return (retval);
}

bool HubAccount::addHubItem(qint64 categoryId, QVariantMap &itemMap, QString name, QString subject,
        qint64 timestamp, QString itemSyncId, QString itemUserData, QString itemExtendedData,
        bool notify)
{
    qint64 retVal = 0;
    bool itemRead = false;
    int itemContextState = 1;

    qDebug() << "add hub item: " << timestamp << " - " << name << " - " << subject;

    retVal = _udsUtil->addItem(_accountId, categoryId, itemMap, name, subject, _itemMimeType,
            _itemUnreadIconFilename, itemRead, itemSyncId, itemUserData, itemExtendedData,
            timestamp, itemContextState, notify);

    if (retVal <= 0) {
        qDebug() << "HubAccount::addHubItem: addItem failed for item: " << name << ", category: "
                << categoryId << ", account: " << _accountId << ", retVal: " << retVal << "\n";
    } else {
        _hubCache->addItem(itemMap);
    }

    return (retVal > 0);
}

bool HubAccount::updateHubItem(qint64 categoryId, qint64 itemId, QVariantMap &itemMap, bool notify)
{
    qint64 retVal = 0;
    int itemContextState = 1;

    qDebug() << "update hub item: " << categoryId << " : " << itemId << " : " << itemMap << " : "
            << notify;

    if ((itemMap["readCount"].toInt() > 0)) {
        retVal = _udsUtil->updateItem(_accountId, categoryId, itemMap, QString::number(itemId),
                itemMap["name"].toString(), itemMap["description"].toString(), _itemMimeType,
                _itemReadIconFilename, (itemMap["readCount"].toInt() > 0),
                itemMap["syncId"].toString(), itemMap["userData"].toString(),
                itemMap["extendedData"].toString(), itemMap["timestamp"].toLongLong(),
                itemContextState, notify);
    } else {
        retVal = _udsUtil->updateItem(_accountId, categoryId, itemMap, QString::number(itemId),
                itemMap["name"].toString(), itemMap["description"].toString(), _itemMimeType,
                _itemUnreadIconFilename, (itemMap["readCount"].toInt() > 0),
                itemMap["syncId"].toString(), itemMap["userData"].toString(),
                itemMap["extendedData"].toString(), itemMap["timestamp"].toLongLong(),
                itemContextState, notify);
    }

    if (retVal <= 0) {
        qDebug() << "HubAccount::updateHubItem: updateItem failed for item: " << itemId
                << ", category: " << categoryId << ", account: " << _accountId << ", retVal: "
                << retVal << "\n";
    } else {
        _hubCache->updateItem(itemId, itemMap);
    }

    return (retVal > 0);
}

bool HubAccount::removeHubItem(qint64 categoryId, qint64 sourceId)
{
    qint64 retVal = 0;

    qDebug() << "remove hub item: " << categoryId << " : " << sourceId;

    retVal = _udsUtil->removeItem(_accountId, categoryId, QString::number(sourceId));
    if (retVal <= 0) {
        qDebug() << "HubAccount::removeHubItem: removeItem failed for item: " << categoryId << " : "
                << sourceId << ", retVal: " << retVal << "\n";
    } else {
        _hubCache->removeItem(sourceId);
    }

    return (retVal > 0);
}

bool HubAccount::markHubItemRead(qint64 categoryId, qint64 itemId)
{
    bool retVal = false;
    qDebug() << "HubAccount::markHubItemRead: " << categoryId << " : " << itemId;

    QVariant* item = getHubItem(categoryId, itemId);

    if (item) {
        QVariantMap itemMap = item->toMap();

        itemMap["readCount"] = 1;
        itemMap["totalCount"] = 1;

        qDebug() << "HubAccount::markHubItemRead: itemMap: " << itemMap;

        retVal = updateHubItem(categoryId, itemId, itemMap, false);
    }

    return retVal;
}

bool HubAccount::markHubItemUnread(qint64 categoryId, qint64 itemId)
{
    bool retVal = false;
    qDebug() << "HubAccount::markHubItemUnread: " << categoryId << " : " << itemId;

    QVariant* item = getHubItem(categoryId, itemId);

    if (item) {
        QVariantMap itemMap = item->toMap();

        itemMap["readCount"] = 0;
        itemMap["totalCount"] = 1;

        qDebug() << "HubAccount::markHubItemUnread: itemMap: " << itemMap;

        retVal = updateHubItem(categoryId, itemId, itemMap, false);
    }

    return retVal;
}

void HubAccount::markHubItemsReadBefore(qint64 categoryId, qint64 timestamp)
{
    qDebug() << "HubAccount::markHubItemsReadBefore: " << categoryId << " : " << timestamp;

    QVariantList _items = items();
    QVariantMap itemMap;
    qint64 itemId;
    qint64 itemCategoryId;
    qint64 itemTime;

    for (int index = 0; index < _items.size(); index++) {
        itemMap = _items.at(index).toMap();
        itemId = itemMap["sourceId"].toLongLong();
        itemCategoryId = itemMap["categoryId"].toLongLong();
        itemTime = itemMap["timestamp"].toLongLong();

        qDebug() << "HubAccount::markHubItemUnread: checking itemMap: " << itemId << " : "
                << itemCategoryId << " : " << itemTime << " : " << itemMap;

        if (itemTime < timestamp && itemCategoryId == categoryId) {
            itemMap["readCount"] = 1;
            updateHubItem(itemCategoryId, itemId, itemMap, false);
            qDebug() << "HubAccount::markHubItemUnread: marking as read: ";
        }
    }
}

void HubAccount::removeHubItemsBefore(qint64 categoryId, qint64 timestamp)
{
    bool retVal = false;

    qDebug() << "HubAccount::removeHubItemsBefore: " << categoryId << " : " << timestamp;

    QVariantList _items;
    QVariantMap itemMap;
    qint64 itemId;
    qint64 itemCategoryId;
    qint64 itemTime;

    bool foundItems = false;
    do {
        foundItems = false;

        _items = items();
        for (int index = 0; index < _items.size(); index++) {
            itemMap = _items.at(index).toMap();
            itemId = itemMap["sourceId"].toLongLong();
            itemCategoryId = itemMap["categoryId"].toLongLong();
            itemTime = itemMap["timestamp"].toLongLong();

            qDebug() << "HubAccount::removeHubItemsBefore: checking itemMap: " << itemId << " : "
                    << itemCategoryId << " : " << itemTime << " : " << itemMap;

            if (itemTime < timestamp && itemCategoryId == categoryId) {
                retVal = removeHubItem(itemCategoryId, itemId);
                if (retVal) {
                    foundItems = true;
                    qDebug() << "HubAccount::removeHubItemsBefore: deleted: ";
                }
                break;
            }
        }
    } while (foundItems);
}

void HubAccount::clearHub()
{
    QVariantList categories = _hubCache->categories();
    QVariantList items = _hubCache->items();

    if (items.size() > 0) {
        for (int index = 0; index < items.size(); index++) {
            QVariantMap itemMap = items.at(index).toMap();

            removeHubItem(itemMap["categoryId"].toLongLong(), itemMap["sourceId"].toLongLong());
        }
    }

    if (categories.size() > 0) {
        for (int index = categories.size() - 1; index >= 0; index--) {
            QVariantMap itemMap = categories.at(index).toMap();

            removeHubCategory(itemMap["categoryId"].toLongLong());
        }
    }

    _udsUtil->restoreNextIds(_accountId + 1, _hubCache->lastCategoryId() + 1,
            _hubCache->lastItemId() + 1);
}

void HubAccount::repopulateHub()
{
    QVariantList categories = _hubCache->categories();
    QVariantList items = _hubCache->items();

    if (categories.size() > 0) {
        for (int index = 0; index < categories.size(); index++) {
            QVariantMap itemMap = categories.at(index).toMap();

            addHubCategory(itemMap["parentCategoryId"].toLongLong(), itemMap["name"].toString());
        }
    }

    if (items.size() > 0) {
        for (int index = 0; index < items.size(); index++) {
            QVariantMap itemMap = items.at(index).toMap();

            addHubItem(itemMap["categoryId"].toLongLong(), itemMap, itemMap["name"].toString(),
                    itemMap["description"].toString(), itemMap["timestamp"].toLongLong(),
                    itemMap["syncId"].toString(), itemMap["userData"].toString(),
                    itemMap["extendedData"].toString(), false);
        }
    }
    qDebug() << "Repopulating Hub Cache -- Found items : " << items.size();
}

void HubAccount::updateAccount(QString type, QString update)
{
    _udsUtil->updateAccount(_hubCache->accountId(), _name, _headlessTarget, _iconFilename,
            _lockedIconFilename, _composeIconFilename, update, false, UDS_ACCOUNT_TYPE_IM);
}
