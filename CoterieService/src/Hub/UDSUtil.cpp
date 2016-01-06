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

#include <QDir>
#include <QThread>
#include <bb/pim/account/Account.hpp>
#include <bb/pim/account/AccountService>
#include <bb/pim/account/Provider>
#include <bb/pim/account/Result>

#include "UDSUtil.hpp"

using namespace bb::pim::account;

UDSUtil::UDSUtil(QString serviceURL, QString hubAssetsFolderName) :
       _nextAccountId(0), _nextCategoryId(0), _nextItemId(0), _async(false), _isInitializationSuccess(false), _isRegistrationSuccess(false), _reloadHub(false) {

    qDebug() << "UDSUtil::UDSUtil: " << serviceURL << " : " << hubAssetsFolderName;

    _async = false;
    _isInitializationSuccess = false;
    _isRegistrationSuccess = false;
    _reloadHub = false;

    // determine perimeter type for inbox items
    char *perimeter = getenv("PERIMETER");
    if (!strcmp(perimeter, "personal")) {
        _itemPerimeterType = UDS_PERIMETER_PERSONAL;
    } else
    if (!strcmp(perimeter, "enterprise")) {
        _itemPerimeterType = UDS_PERIMETER_ENTERPRISE;
    }

    memset(_serviceURL, 0, 256);
    strncpy(_serviceURL, serviceURL.toUtf8().data(), 255);

    // determine absolute path for hub assets
    QString tmpPath = QDir::current().absoluteFilePath("data");
    tmpPath = tmpPath.replace("/data", "/public/");
    tmpPath = tmpPath.append(hubAssetsFolderName);
    tmpPath = tmpPath.append("/");
    if (!strcmp(perimeter, "personal")) {
        tmpPath = tmpPath.replace("/accounts/1000/appdata", "/apps");
    } else
    if (!strcmp(perimeter, "enterprise")) {
        tmpPath = tmpPath.replace("/accounts/1000-enterprise/appdata", "/apps-enterprise");
    }

    memset(_assetPath, 0, 256);
    strcpy(_assetPath, tmpPath.toUtf8().data());
    qDebug() << "UDSUtil::UDSUtil: assetPath: " << _assetPath;
}

UDSUtil::~UDSUtil() {
}

void UDSUtil::initialize() {
    const char* libPath = "";
    int retVal = -1;
    bool retval;
    int serviceId = 0;
    int status = 0;

    if (!_isRegistrationSuccess) {
        do {
            if (!_isInitializationSuccess) {
                retVal = uds_init(&_udsHandle, _async);
                if (retVal == UDS_SUCCESS) {
                    qDebug() << "UDSUtil::initialize: uds_init: successful\n";
                    _isInitializationSuccess = true;
                }
            } else {
                retVal = UDS_SUCCESS; // already initialized ... retrying registration
            }

            if (retVal == UDS_SUCCESS) {
                if ((retVal = uds_register_client(_udsHandle, _serviceURL, libPath, _assetPath)) != 0) {
                    qCritical() << "UDSUtil::initialize: uds_register_client call failed with error " << retVal;
                }
                if (retVal == UDS_ERROR_TIMEOUT) {
                    qWarning() << "UDSUtil::initialize: uds_register_client timed out: retrying shortly ...";
                } else {
                    // not sure if this is better than the check below
                    if (retVal == UDS_REGISTRATION_NEW) {
                        _reloadHub = true;
                        qDebug() << "UDSUtil::initialize: uds_register_client call return code indicates Hub reload required.";
                    }

                    retval = uds_wait_for_response(_udsHandle, 300000);
                    if (_async) {
                        retVal = uds_get_response(_udsHandle);
                        if (retVal == 0) {
                            serviceId = uds_get_service_id(_udsHandle);
                            status = uds_get_service_status(_udsHandle);
                        }
                    } else {
                        if (retval) {
                            serviceId = uds_get_service_id(_udsHandle);
                            status = uds_get_service_status(_udsHandle);
                        }
                    }
                    qDebug() << "UDSUtil::initialize: uds_register_client call successful with " << serviceId << " as serviceId and " << status << " as status\n";
                    if (retval || retVal == 0)
                        _isRegistrationSuccess = true;

                    if (status == UDS_REGISTRATION_NEW) {
                        _reloadHub = true;
                        qDebug() << "UDSUtil::initialize: uds_get_service_status call return code indicates Hub reload required.";
                    }

                    initNextIds();

                    break;
                }
            } else if (retVal == UDS_ERROR_TIMEOUT) {
                qWarning() << "UDSUtil::initialize: uds_init: timed out: retrying shortly ...";
            } else {
                qCritical() << "UDSUtil::initialize: uds_init: failed: " << retVal;
                break;
            }
        } while (true);
    }
}

void UDSUtil::initNextIds() {
    _nextAccountId = 0;
    _nextCategoryId = 1;
    _nextItemId = 1;
}

bool UDSUtil::initialized() {
    return _isInitializationSuccess;
}

bool UDSUtil::registered() {
    return _isRegistrationSuccess;
}

bool UDSUtil::reloadHub() {
    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::reloadHub: not registered yet\n";
        return false;
    }

    return _reloadHub;
}

void UDSUtil::resetReloadHub() {
    _reloadHub = false;
}

bool UDSUtil::restoreNextIds(qint64 nextAccountId, qint64 nextCategoryId, qint64 nextItemId) {
    bool retVal = true;

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::restoreNextIds: not registered yet\n";
        return retVal;
    }

    _nextAccountId  = nextAccountId;
    _nextCategoryId = nextCategoryId;
    _nextItemId     = nextItemId;

    return retVal;
}

qint64 UDSUtil::addAccount(QString name, QString displayName, QString serverName, QString target,
						QString icon, QString lockedIcon, QString composeIcon, QString desc,
						bool compose, uds_account_type_t type)
{
    int retCode = -1;
    uds_account_key_t accountId;
    char accountName[256];
    char targetName[256];
    char iconName[256];
    char lockedIconName[256];
    char composeIconName[256];
    char description[256];

    Account act;
    AccountService accountService;
    QString s;
    char *p = 0;

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::addAccount: not registered yet\n";
        return -1;
    }

    memset(accountName, 0, 256);
    strncpy(accountName, name.toUtf8().data(), 255);
    memset(targetName, 0, 256);
    strncpy(targetName, target.toUtf8().data(), 255);
    memset(iconName, 0, 256);
    strncpy(iconName, icon.toUtf8().data(), 255);
    memset(lockedIconName, 0, 256);
    strncpy(lockedIconName, lockedIcon.toUtf8().data(), 255);
    memset(composeIconName, 0, 256);
    strncpy(composeIconName, composeIcon.toUtf8().data(), 255);
    memset(description, 0, 256);
    strncpy(description, desc.toUtf8().data(), 255);

    if (_itemPerimeterType == UDS_PERIMETER_ENTERPRISE) {
        displayName.append("-Work");
    }

	// check for existing account and use that accountID as a starting point
	QList<Account> allAccounts;
	do {
		allAccounts = accountService.accounts();
		qDebug() <<  "UDSUtil::addAccount: # accounts: " << allAccounts.length();

		if (allAccounts.length() > 0) {
		   for(int index = 0; index < allAccounts.length(); index++) {
			   Account account = allAccounts.at(index);

			   //char *accountName = (char *)account.displayName().toUtf8().constData();
			   qDebug() << "UDSUtil: addAccount: check account " << index << " : " << account.displayName() << account.isServiceSupported(Service::Messages) << " : " << account.provider().id();

		       if (_itemPerimeterType == UDS_PERIMETER_ENTERPRISE) {
		            account.setExternalEnterprise (Property::Enterprise);
                   if (account.displayName() == displayName && account.provider().id() == "external" && account.isEnterprise() == Property::Enterprise) {
                       _nextAccountId = account.id();
                       qDebug() << "UDSUtil: addAccount: found existing account " << _nextAccountId;
                   }
		       } else {
                   if (account.displayName() == displayName && account.provider().id() == "external") {
                       _nextAccountId = account.id();
                       qDebug() << "UDSUtil: addAccount: found existing account " << _nextAccountId;
                   }
               }
			}
		}
	} while (allAccounts.length() == 0);

    // create the message service account
	if (_nextAccountId != 0) {
	    cleanupAccountsExcept(_nextAccountId, displayName);

        accountService.updateAccount (act.id(), act);
	} else {
		QString providerId("external");  // this maps to the filename of the provider's json file
		const Provider provider = accountService.provider(providerId);
		Account account(provider);
		account.setExternalData(true);
		if (_itemPerimeterType == UDS_PERIMETER_ENTERPRISE) {
		    account.setExternalEnterprise (Property::Enterprise);
		}
		account.setSettingsValue("server", serverName);
		account.setDisplayName(displayName);
		Result r = accountService.createAccount(provider.id(), account);
		if (!r.isSuccess())
		{
			s = r.message();
			p = s.toUtf8().data();
			qCritical() << "UDSUtil::addAccount: account create failed !"<<s<<p<<account.isValid();
			act = Account();
		}
		else
		{
			// Was successful, so lets get the account id
			AccountKey id = account.id();
			qDebug() << "UDSUtil::addAccount: account created with id "<<id<<account.isValid();
			act = account;
		}

		if( act.isValid() ){
			_nextAccountId = act.id();
		}else{
			_nextAccountId = 0;
		}
    }

    qDebug() << "UDSUtil: addAccount: _udsHandle " << _udsHandle;

	// create the UDS account
    uds_account_data_t *accountData = uds_account_data_create();
    uds_account_data_set_id(accountData,_nextAccountId);
    uds_account_data_set_name(accountData,accountName);
    uds_account_data_set_icon(accountData, iconName);
    uds_account_data_set_locked_icon(accountData,lockedIconName);
    uds_account_data_set_description(accountData, description);
    uds_account_data_set_compose_icon(accountData, composeIconName);
    uds_account_data_set_supports_compose(accountData,compose);
    uds_account_data_set_type(accountData, type);
    uds_account_data_set_target_name(accountData, targetName);

    if (0 != (retCode = uds_account_added(_udsHandle, accountData))) {
        qCritical() << "UDSUtil::addAccount:  uds_account_added failed with error " << retCode << " for account: " << name << "\n";
        accountId = -1;
    } else {
        if (_nextAccountId == 0) {
            _nextAccountId++;
        }
    	accountId = _nextAccountId;
        //mapAccount[name] = uds_account_data_get_id(accountData);
        qDebug() << "UDSUtil::addAccount: account: " << name << ": added, id" << QString::number(_nextAccountId);
        _nextAccountId++;
    }

    if (_async) {
        retCode = uds_get_response(_udsHandle);
    }

    uds_account_data_destroy(accountData);

    return accountId;
}

bool UDSUtil::updateAccount(qint64 accountId, QString name, QString target,
							QString icon, QString lockedIcon, QString composeIcon, QString desc,
							bool compose, uds_account_type_t type)
{
	bool retVal = false;
	char accountName[256];
	char targetName[256];
	char iconName[256];
	char lockedIconName[256];
	char composeIconName[256];
	char description[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::updateAccount: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || name.size() == 0 || target.size() == 0 || icon.size() == 0 || lockedIcon.size() == 0 || composeIcon.size() == 0 || type == 0) {
        qCritical() << "UDSUtil::updateAccount: invalid parameters\n";
        return retVal;
    }

    memset(accountName, 0, 256);
    strncpy(accountName, name.toUtf8().data(), 255);
    memset(targetName, 0, 256);
    strncpy(targetName, target.toUtf8().data(), 255);
    memset(iconName, 0, 256);
    strncpy(iconName, icon.toUtf8().data(), 255);
    memset(lockedIconName, 0, 256);
    strncpy(lockedIconName, lockedIcon.toUtf8().data(), 255);
    memset(composeIconName, 0, 256);
    strncpy(composeIconName, composeIcon.toUtf8().data(), 255);
    memset(description, 0, 256);
    strncpy(description, desc.toUtf8().data(), 255);

    uds_account_data_t *accountData = uds_account_data_create();
    uds_account_data_set_id(accountData,accountId);
    uds_account_data_set_name(accountData,accountName);
    uds_account_data_set_icon(accountData, iconName);
    uds_account_data_set_locked_icon(accountData,lockedIconName);
    uds_account_data_set_description(accountData, description);
    uds_account_data_set_compose_icon(accountData, composeIconName);
    uds_account_data_set_supports_compose(accountData,compose);
    uds_account_data_set_type(accountData, type);
    uds_account_data_set_target_name(accountData, targetName);

    if (0 != uds_account_updated(_udsHandle, accountData)) {
        qCritical() << "UDSUtil::updateAccount: uds_account_updated failed with error " << retVal << " for account: " << name << "\n";
        retVal = false;
    } else {
        qDebug() << "UDSUtil::updateAccount: account: " << accountId << " updated";
        retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }

    uds_account_data_destroy(accountData);

    return retVal;
}

bool UDSUtil::removeAccount(qint64 accountId) {
    bool retVal = true;

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::removeAccount: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0) {
        qCritical() << "UDSUtil::removeAccount: invalid parameters\n";
        return retVal;
    }

    if (0 != (uds_account_removed(_udsHandle, accountId))) {
        qCritical() << "UDSUtil::removeAccount: uds_account_removed for " << " + accountId "
        		"+ " << " failed with error " << retVal << "\n";
        retVal = false;
    } else {
    	qDebug() << "UDSUtil::removeAccount: account " << accountId << " removed";
    	retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }

    return retVal;
}

void UDSUtil::cleanupAccountsExcept(const qint64 accountId, const QString& name)
{
	qDebug() << "UDSUtil::cleanupAccountsExcept : name : " << name << " : accountId: " << accountId;

	int accRemoved = 0;

	if (accountId != -1) {
		QString s;
		char *p = 0;

		Account act;
		AccountService accountService;

		QString providerId("external");
		const Provider provider = accountService.provider(providerId);

		QList<Account> allAccounts;
		do {
			allAccounts = accountService.accounts();
			qDebug() <<  "UDSUtil::cleanupAccountsExcept: # accounts: " << allAccounts.length();

			if (allAccounts.length() > 0) {
				//JsonDataAccess jda;


			   for(int index = 0; index < allAccounts.length(); index++) {
				   Account account = allAccounts.at(index);
				   QByteArray rawBuffer;

				   if (account.displayName() == name) {
					   qDebug() << "UDSUtil: cleanupAccountsExcept: account " << index
							   << "\n : Account ID: " << account.id()
							   << "\n : Account Name: " << account.displayName()
							   << "\n : Invoke Target:  " <<  account.externalSetupInvokeTarget()
							   << "\n : Messages supported: " << account.isServiceSupported(Service::Messages)
							   << "\n : Provider ID: " << account.provider().id();

					   qint64 accId = account.id();

						if (accId != accountId) {
							removeAccount(accId);

							Result r = accountService.deleteAccount(accId);
							qDebug() << "UDSUtil::cleanupAccountsExcept : Removing account...";
							if (!r.isSuccess())
							{
								s = r.message();
								p = s.toUtf8().data();
								qCritical() << "UDSUtil::cleanupAccountsExcept : account remove failed !"<<s<<p<<account.isValid();
								act = Account();
							}
							else
							{
								// Was successful, so lets get the account id
								qDebug() << "UDSUtil::cleanupAccountsExcept: account with id "<<accId<< " removed";
								act = account;
								accRemoved++;
							}
						}
				   }
				}
			}

			// in case service does not start up right away, retry until it becomes available
			//usleep(5000);
		} while (allAccounts.length() == 0);
	} else {
		qCritical() << "UDSUtil::cleanupAccountsExcept : No account id, aborting.";
	}

	qDebug() << "UDSUtil::cleanupAccountsExcept: Accounts Removed: " << accRemoved;
}

qint64 UDSUtil::addCategory(qint64 accountId, QString name, qint64 parentCategoryId = 0) {
    int retVal = -1;
    int retCode = 0;
    char categoryName[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::addCategory: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || name.size() == 0) {
        qCritical() << "UDSUtil::addCategory: invalid parameters\n";
        return retVal;
    }

    memset(categoryName, 0, 256);
    strcpy(categoryName, name.toUtf8().data());

    uds_category_data_t *category = uds_category_data_create();
    uds_category_data_set_id(category, _nextCategoryId);
    uds_category_data_set_parent_id(category, parentCategoryId);
    uds_category_data_set_account_id(category, accountId);
    uds_category_data_set_name(category, categoryName);

    if (0 != (retCode = uds_category_added(_udsHandle, category))) {
        qCritical() << "UDSUtil::addCategory: uds_category_added failed for  " << categoryName << " with error " << retCode << "\n";
        retVal = -1;
    } else {
        qDebug() << "UDSUtil::addCategory: category: " << _nextCategoryId << ":  " << name << ": added";
        retVal = _nextCategoryId;
        _nextCategoryId++;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
        if (retVal == 0) {
            retVal = _nextCategoryId;
            _nextCategoryId++;
        } else {
        	retVal = 0;
        }
    }

    uds_category_data_destroy(category);

    return retVal;
}

bool UDSUtil::updateCategory(qint64 accountId, qint64 categoryId, QString name, qint64 parentCategoryId = 0) {
    bool retVal = true;
    int retCode = 0;
    char categoryName[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::updateCategory: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || categoryId <= 0 || name.size() == 0) {
        qCritical() << "UDSUtil::updateCategory: invalid parameters\n";
        return retVal;
    }

    memset(categoryName, 0, 256);
    strcpy(categoryName, name.toUtf8().data());

    uds_category_data_t *category = uds_category_data_create();
    uds_category_data_set_id(category,  categoryId);
    uds_category_data_set_parent_id(category, parentCategoryId);
    uds_category_data_set_account_id(category, accountId);
    uds_category_data_set_name(category, categoryName);


    if (0 != (retCode = uds_category_updated(_udsHandle, category))) {
        qCritical() << "UDSUtil::updateCategory: uds_category_updated failed for " << name << " with error "<< retCode << "\n";
        retVal = false;
    } else {
        qDebug() << "UDSUtil::updateCategory: category: " << categoryId << ": updated";

        retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }

    return retVal;
}

bool UDSUtil::removeCategory(qint64 accountId, qint64 categoryId) {
    bool retVal = true;

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::removeCategory: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || categoryId <= 0) {
        qCritical() << "UDSUtil::removeCategory: invalid parameters\n";
        return retVal;
    }

    if (0 != (uds_category_removed(_udsHandle, accountId, categoryId))) {
    	qCritical() << "UDSUtil::removeCategory:: uds_category_removed failed with error " << retVal << "\n";
    	retVal = false;
    } else {
    	qDebug() << "UDSUtil::removeCategory: category: " << categoryId << ": removed";
    	retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }

    return retVal;
}

qint64 UDSUtil::addItem(qint64 accountId, qint64 categoryId, QVariantMap &itemMap, QString name, QString subject, QString mimeType,
                        QString icon, bool read, QString syncId, QString userData, QString extendedData,
					    long long timestamp, unsigned int contextState, bool notify) {
    int retVal = -1;
    int retCode = -1;
    char sourceId[256];
    char itemName[256];
    char subjectDesc[256];
    char mime[256];
    char iconName[256];
    char syncIdS[256];
    char usrData[2048];
    char extendData[2048];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::addItem: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || categoryId <= 0 || name.size() == 0 || subject.size() == 0 || mimeType.size() == 0 || icon.size() == 0 || syncId.size() == 0 || timestamp <= 0) {
        qCritical() << "UDSUtil::addItem: invalid parameters\n";
        return retVal;
    }

    memset(itemName, 0, 256);
    strcpy(itemName,name.toUtf8().data());
    memset(subjectDesc, 0, 256);
    strcpy(subjectDesc, subject.toUtf8().data());
    memset(mime, 0, 256);
    strcpy(mime, mimeType.toUtf8().data());
    memset(iconName, 0, 256);
    strcpy(iconName, icon.toUtf8().data());
    memset(syncIdS, 0, 256);
    strcpy(syncIdS, syncId.toUtf8().data());
    memset(usrData, 0, 2048);
    strcpy(usrData, userData.toUtf8().data());
    memset(extendData, 0, 2048);
    strcpy(extendData, extendedData.toUtf8().data());

    itoa((_nextItemId), sourceId, 10);
    QString sourceId1 = QString::fromStdString(sourceId);

    uds_inbox_item_data_t *inbox_item = uds_inbox_item_data_create();

    qDebug() << "UDSUtil::addItem: item " << itemName << " : " << subjectDesc << " : " << iconName << " : " << mime << " : " << accountId << " : " << categoryId << " : " << sourceId << "\n";

    uds_inbox_item_data_set_name(inbox_item, itemName);
    uds_inbox_item_data_set_description(inbox_item, subjectDesc);
    uds_inbox_item_data_set_icon(inbox_item,iconName);
    uds_inbox_item_data_set_mime_type(inbox_item,mime);
    uds_inbox_item_data_set_unread_count(inbox_item, read ? 0 : 1);
    uds_inbox_item_data_set_total_count(inbox_item,1);
    uds_inbox_item_data_set_source_id(inbox_item,sourceId);
    uds_inbox_item_data_set_sync_id(inbox_item, (char *)syncIdS);
    uds_inbox_item_data_set_account_id(inbox_item, accountId);
    uds_inbox_item_data_set_user_data(inbox_item, usrData);
    uds_inbox_item_data_set_extended_data(inbox_item, extendData);
    uds_inbox_item_data_set_category_id(inbox_item, categoryId);
    uds_inbox_item_data_set_timestamp(inbox_item,timestamp);
    uds_inbox_item_data_set_context_state(inbox_item, contextState);
    uds_inbox_item_data_set_notification_state(inbox_item,notify);
    uds_inbox_item_data_set_perimeter(inbox_item, _itemPerimeterType);

    itemMap["accountId"] = accountId;
    itemMap["categoryId"] = categoryId;
    itemMap["name"] = name;
    itemMap["description"] = subject;
    itemMap["mimeType"] = mimeType;
    itemMap["icon"] = icon;
    itemMap["readCount"] = read ? 0 : 1;
    itemMap["totalCount"] = 1;
    itemMap["syncId"] = syncId;
    itemMap["timestamp"] = timestamp;
    itemMap["contextState"] = contextState;
    itemMap["notify"] = notify;
    itemMap["sourceId"] = sourceId;
    itemMap["userData"] = userData;
    itemMap["extendedData"] = extendedData;

    if (0 != (retCode = uds_item_added(_udsHandle, inbox_item))) {
        qCritical() << "UDSUtil::addItem: uds_item_added failed for " << name << " with error "<< retCode << "\n";
        retVal = 0;
    } else {
        qDebug() << "UDSUtil::addItem: item: " << itemMap << ": added";
        retVal = _nextItemId;
        _nextItemId++;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
        if (retVal == 0) {
            retVal = _nextItemId;
            _nextItemId++;
        } else {
        	retVal = 0;
        }
    }
    uds_inbox_item_data_destroy(inbox_item);

    return retVal;
}

bool UDSUtil::updateItem(qint64 accountId, qint64 categoryId, QVariantMap &itemMap, QString srcId, QString name, QString subject, QString mimeType, QString icon, bool read,
                            QString syncId, QString userData, QString extendedData,
                            long long timestamp, unsigned int contextState, bool notify) {
    bool retVal = true;
    int retCode = -1;
    char sourceId[256];
    char itemName[256];
    char subjectDesc[256];
    char mime[256];
    char iconName[256];
    char syncIdS[256];
    char usrData[2048];
    char extendData[2048];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::updateItem: not registered yet\n";
        return retVal;
    }

    qDebug() << "UDSUtil::updateItem: item " << accountId << " : " << srcId << " : " << name << " : " << subject << " : " << mimeType << " : " << icon << " : " << syncId << " : " << timestamp << "\n";

    if (accountId <= 0 || categoryId <= 0 || srcId.size() == 0 || name.size() == 0 || subject.size() == 0 || mimeType.size() == 0 || icon.size() == 0 || syncId.size() == 0 || timestamp <= 0) {
        qCritical() << "UDSUtil::updateItem: invalid parameters\n";
        return retVal;
    }

    memset(itemName, 0, 256);
    strcpy(itemName,name.toUtf8().data());
    memset(subjectDesc, 0, 256);
    strcpy(subjectDesc, subject.toUtf8().data());
    memset(mime, 0, 256);
    strcpy(mime, mimeType.toUtf8().data());
    memset(iconName, 0, 256);
    strcpy(iconName, icon.toUtf8().data());
    memset(sourceId, 0, 256);
    strcpy(sourceId, srcId.toUtf8().data());
    memset(syncIdS, 0, 256);
    strcpy(syncIdS, syncId.toUtf8().data());
    memset(usrData, 0, 2048);
    strcpy(usrData, userData.toUtf8().data());
    memset(extendData, 0, 2048);
    strcpy(extendData, extendedData.toUtf8().data());

    qDebug() << "UDSUtil::updateItem: item " << itemName << " : " << subjectDesc << " : " << iconName << " : " << mime << " : " << accountId << " : " << categoryId << " : " << sourceId << "\n";

    uds_inbox_item_data_t *inbox_item = uds_inbox_item_data_create();

    uds_inbox_item_data_set_name(inbox_item, itemName);
    uds_inbox_item_data_set_description(inbox_item, subjectDesc);
    uds_inbox_item_data_set_icon(inbox_item,iconName);
    uds_inbox_item_data_set_mime_type(inbox_item,mime);
    uds_inbox_item_data_set_unread_count(inbox_item, read ? 0 : 1);
    uds_inbox_item_data_set_total_count(inbox_item,1);
    uds_inbox_item_data_set_source_id(inbox_item,sourceId);
    uds_inbox_item_data_set_sync_id(inbox_item, (char *)syncIdS);
    uds_inbox_item_data_set_user_data(inbox_item, usrData);
    uds_inbox_item_data_set_extended_data(inbox_item, extendData);
    uds_inbox_item_data_set_account_id(inbox_item, accountId);
    uds_inbox_item_data_set_category_id(inbox_item, categoryId);
    uds_inbox_item_data_set_timestamp(inbox_item,timestamp);
    uds_inbox_item_data_set_context_state(inbox_item, contextState);
    uds_inbox_item_data_set_notification_state(inbox_item,notify);

    itemMap["accountId"] =  accountId;
    itemMap["categoryId"] = categoryId;
    itemMap["name"] = name;
    itemMap["description"] = subject;
    itemMap["mimeType"] = mimeType;
    itemMap["icon"] = icon;
    itemMap["readCount"] = read ? 0 : 1;
    itemMap["totalCount"] = 1;
    itemMap["syncId"] = syncId;
    itemMap["timestamp"] = timestamp;
    itemMap["contextState"] = contextState;
    itemMap["notify"] = notify;
    itemMap["sourceId"] = sourceId;
    itemMap["userData"] = userData;
    itemMap["extendedData"] = extendedData;

    if (0 != (retCode = uds_item_updated(_udsHandle, inbox_item))) {
        qCritical() << "UDSUtil::updateItem: uds_item_updated failed for " << name << " with error " << retCode << "\n";
        retVal = false;
    } else {
        qDebug() << "UDSUtil::updateItem: item " << sourceId << " updated";

        retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }
    uds_inbox_item_data_destroy(inbox_item);

    return retVal;
}

bool UDSUtil::removeItem(qint64 accountId, qint64 categoryId, QString srcId) {
    bool retVal = true;
    char sourceId[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::removeItem: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || categoryId <= 0 || srcId.size() == 0) {
        qCritical() << "UDSUtil::removeItem:: invalid parameters\n";
        return retVal;
    }

    strcpy(sourceId, srcId.toUtf8().data());

    if (0 != (uds_item_removed(_udsHandle, accountId, sourceId))) {
    	qCritical() << "UDSUtil::removeItem: uds_item_removed for " << sourceId << " failed with error " << retVal << "\n";
    	retVal = false;
    } else {
    	qDebug() << "UDSUtil::removeItem: item: " << sourceId << ": removed";

    	retVal = true;
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }

    return retVal;
}

bool UDSUtil::addAccountAction(qint64 accountId, QString action, QString title,
								QString invtarget, QString invtargettype, QString imgsource,
								QString mimetype, int placement) {

    bool retval = true;
    int retVal = -1;
    char accAction[256];
    char accTitle[256];
    char actionTarget[256];
    char actionTargetType[256];
    char mimeType[256];
    char imgSource[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::addAccountActionData: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || action.size() == 0|| title.size() == 0 || invtarget.size() == 0 || invtargettype.size() == 0 || mimetype.size() == 0 || placement <= 0) {
        qCritical() << "UDSUtil::addAccountActionData: invalid parameters\n";
        return retVal;
    }

    memset(accAction, 0, 256);
    strcpy(accAction, action.toUtf8().data());
    memset(accTitle, 0, 256);
    strcpy(accTitle, title.toUtf8().data());
    memset(actionTarget, 0, 256);
    strcpy(actionTarget, invtarget.toUtf8().data());
    memset(actionTargetType, 0, 256);
    strcpy(actionTargetType, invtargettype.toUtf8().data());
    memset(imgSource, 0, 256);
    strcpy(imgSource, imgsource.toUtf8().data());
    memset(mimeType, 0, 256);
    strcpy(mimeType, mimetype.toUtf8().data());

    uds_account_action_data_t * account_action = uds_account_action_data_create();

    uds_account_action_data_set_action(account_action,accAction);
    uds_account_action_data_set_title(account_action,accTitle);
    uds_account_action_data_set_target(account_action,actionTarget);
    uds_account_action_data_set_type(account_action,actionTargetType);
    uds_account_action_data_set_mime_type(account_action,mimeType);
    uds_account_action_data_set_image_source(account_action,imgSource);
    uds_account_action_data_set_placement(account_action,(uds_placement_type_t)placement);

    if (0 != (retVal = uds_register_account_action(_udsHandle, accountId, account_action))) {
        retval = false;
        qCritical() << "uds_register_account_action failed with error " <<  retVal << "\n";
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }
    uds_account_action_data_destroy(account_action);

    return retval;

}

bool UDSUtil::updateAccountAction(qint64 accountId, QString action, QString title,
								QString invtarget, QString invtargettype, QString imgsource,
								QString mimetype, int placement) {

    bool retval = true;
	int retVal = -1;
	char accAction[256];
	char accTitle[256];
	char actionTarget[256];
	char actionTargetType[256];
	char mimeType[256];
	char imgSource[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::updateAccountActionData: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || action.size() == 0|| title.size() == 0 || invtarget.size() == 0 || invtargettype.size() == 0 || mimetype.size() == 0 || placement <= 0) {
        qCritical() << "UDSUtil::updateAccountActionData: invalid parameters\n";
        return retVal;
    }

    memset(accAction, 0, 256);
    strcpy(accAction, action.toUtf8().data());
    memset(accTitle, 0, 256);
    strcpy(accTitle, title.toUtf8().data());
    memset(actionTarget, 0, 256);
    strcpy(actionTarget, invtarget.toUtf8().data());
    memset(actionTargetType, 0, 256);
    strcpy(actionTargetType, invtargettype.toUtf8().data());
    memset(imgSource, 0, 256);
    strcpy(imgSource, imgsource.toUtf8().data());
    memset(mimeType, 0, 256);
    strcpy(mimeType, mimetype.toUtf8().data());

    uds_account_action_data_t * account_action = uds_account_action_data_create();
    uds_account_action_data_set_action(account_action,accAction);
    uds_account_action_data_set_title(account_action,accTitle);
    uds_account_action_data_set_target(account_action,actionTarget);
    uds_account_action_data_set_type(account_action,actionTargetType);
    uds_account_action_data_set_image_source(account_action,imgSource);
    uds_account_action_data_set_mime_type(account_action,mimeType);
    uds_account_action_data_set_placement(account_action,(uds_placement_type_t)placement);

    if (0 != (retVal = uds_update_account_action(_udsHandle, accountId, account_action))) {
        retval = false;
        qCritical() << "uds_update_account_action failed with error " << retVal << "\n";
    } else {
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }
    uds_account_action_data_destroy(account_action);

    return retval;
}

bool UDSUtil::addItemAction(qint64 accountId, QString action,
                            QString title, QString invtarget, QString invtargettype,
                            QString imgsource, QString mimetype, int placement) {

    bool retval = true;
    int retVal = -1;
	char itemAction[256];
	char itemActionTitle[256];
	char itemActionTarget[256];
	char itemActionTargetType[256];
	char mimeType[256];
	char imgSource[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::addItemActionData: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || action.size() == 0|| title.size() == 0 || invtarget.size() == 0 || invtargettype.size() == 0 || imgsource.size() == 0 || mimetype.size() == 0 || placement <= 0) {
        qCritical() << "UDSUtil::addItemActionData: invalid parameters\n";
        return retVal;
    }

    memset(itemAction, 0, 256);
    strcpy(itemAction, action.toUtf8().data());
    memset(itemActionTitle, 0, 256);
    strcpy(itemActionTitle, title.toUtf8().data());
    memset(itemActionTarget, 0, 256);
    strcpy(itemActionTarget, invtarget.toUtf8().data());
    memset(itemActionTargetType, 0, 256);
    strcpy(itemActionTargetType, invtargettype.toUtf8().data());
    memset(mimeType, 0, 256);
    strcpy(mimeType, mimetype.toUtf8().data());
    memset(imgSource, 0, 256);
    strcpy(imgSource, imgsource.toUtf8().data());

    uds_item_action_data_t *item_action = uds_item_action_data_create();
    uds_item_action_data_set_action(item_action,itemAction);
    uds_item_action_data_set_title(item_action,itemActionTitle);
    uds_item_action_data_set_target(item_action,itemActionTarget);
    uds_item_action_data_set_type(item_action,itemActionTargetType);
    uds_item_action_data_set_mime_type(item_action,mimeType);
    uds_item_action_data_set_image_source(item_action,imgSource);
    uds_item_action_data_set_placement(item_action,(uds_placement_type_t)placement);
    uds_item_action_data_set_context_mask(item_action,1);

    if (0 != (retVal = uds_register_item_context_action(_udsHandle,
                    accountId, item_action))) {
        retval = false;
        qCritical() << "uds_register_item_context_action failed with error " << retVal << "\n";
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }
    uds_item_action_data_destroy(item_action);

    return retval;

}

bool UDSUtil::updateItemAction(qint64 accountId, QString action,
                            QString title, QString invtarget, QString invtargettype,
                            QString imgsource, QString mimetype, int placement) {

    bool retval = true;
    int retVal = -1;
	char itemAction[256];
	char itemActionTitle[256];
	char itemActionTarget[256];
	char itemActionTargetType[256];
	char mimeType[256];
	char imgSource[256];

    if (!_isRegistrationSuccess) {
        qCritical() << "UDSUtil::updateItemActionData: not registered yet\n";
        return retVal;
    }

    if (accountId <= 0 || action.size() == 0|| title.size() == 0 || invtarget.size() == 0 || invtargettype.size() == 0 || imgsource.size() == 0 || mimetype.size() == 0 || placement <= 0) {
        qCritical() << "UDSUtil::updateItemActionData: invalid parameters\n";
        return retVal;
    }

    memset(itemAction, 0, 256);
    strcpy(itemAction, action.toUtf8().data());
    memset(itemActionTitle, 0, 256);
    strcpy(itemActionTitle, title.toUtf8().data());
    memset(itemActionTarget, 0, 256);
    strcpy(itemActionTarget, invtarget.toUtf8().data());
    memset(itemActionTargetType, 0, 256);
    strcpy(itemActionTargetType, invtargettype.toUtf8().data());
    memset(mimeType, 0, 256);
    strcpy(mimeType, mimetype.toUtf8().data());
    memset(imgSource, 0, 256);
    strcpy(imgSource, imgsource.toUtf8().data());

    uds_item_action_data_t *item_action = uds_item_action_data_create();
    uds_item_action_data_set_action(item_action,itemAction);
    uds_item_action_data_set_title(item_action,itemActionTitle);
    uds_item_action_data_set_target(item_action,itemActionTarget);
    uds_item_action_data_set_type(item_action,itemActionTargetType);
    uds_item_action_data_set_mime_type(item_action,mimeType);
    uds_item_action_data_set_image_source(item_action,imgSource);
    uds_item_action_data_set_placement(item_action,(uds_placement_type_t)placement);
    uds_item_action_data_set_context_mask(item_action,1);

    if (0 != (retVal = uds_update_item_context_action(_udsHandle,
            accountId, item_action))) {
        retval = false;
        qCritical() << "uds_update_item_context_action failed with error " << retVal << "\n";
    }

    if (_async) {
        retVal = uds_get_response(_udsHandle);
    }
    uds_item_action_data_destroy(item_action);

    return retval;
}
