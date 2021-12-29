/**
 * Port Drayage Web Service.
 * Web Service for Loading/Unloading/Inspection interactions for Port Drayage Operations.
 *
 * The version of the OpenAPI document: 1.0
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

#include "OAIInspectionRequest.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QObject>

#include "OAIHelpers.h"

namespace OpenAPI {

OAIInspectionRequest::OAIInspectionRequest(QString json) {
    this->initializeModel();
    this->fromJson(json);
}

OAIInspectionRequest::OAIInspectionRequest() {
    this->initializeModel();
}

OAIInspectionRequest::~OAIInspectionRequest() {}

void OAIInspectionRequest::initializeModel() {

    m_vehicle_id_isSet = false;
    m_vehicle_id_isValid = false;

    m_container_id_isSet = false;
    m_container_id_isValid = false;

    m_action_id_isSet = false;
    m_action_id_isValid = false;
}

void OAIInspectionRequest::fromJson(QString jsonString) {
    QByteArray array(jsonString.toStdString().c_str());
    QJsonDocument doc = QJsonDocument::fromJson(array);
    QJsonObject jsonObject = doc.object();
    this->fromJsonObject(jsonObject);
}

void OAIInspectionRequest::fromJsonObject(QJsonObject json) {

    m_vehicle_id_isValid = ::OpenAPI::fromJsonValue(vehicle_id, json[QString("vehicle_id")]);
    m_vehicle_id_isSet = !json[QString("vehicle_id")].isNull() && m_vehicle_id_isValid;

    m_container_id_isValid = ::OpenAPI::fromJsonValue(container_id, json[QString("container_id")]);
    m_container_id_isSet = !json[QString("container_id")].isNull() && m_container_id_isValid;

    m_action_id_isValid = ::OpenAPI::fromJsonValue(action_id, json[QString("action_id")]);
    m_action_id_isSet = !json[QString("action_id")].isNull() && m_action_id_isValid;
}

QString OAIInspectionRequest::asJson() const {
    QJsonObject obj = this->asJsonObject();
    QJsonDocument doc(obj);
    QByteArray bytes = doc.toJson();
    return QString(bytes);
}

QJsonObject OAIInspectionRequest::asJsonObject() const {
    QJsonObject obj;
    if (m_vehicle_id_isSet) {
        obj.insert(QString("vehicle_id"), ::OpenAPI::toJsonValue(vehicle_id));
    }
    if (m_container_id_isSet) {
        obj.insert(QString("container_id"), ::OpenAPI::toJsonValue(container_id));
    }
    if (m_action_id_isSet) {
        obj.insert(QString("action_id"), ::OpenAPI::toJsonValue(action_id));
    }
    return obj;
}

QString OAIInspectionRequest::getVehicleId() const {
    return vehicle_id;
}
void OAIInspectionRequest::setVehicleId(const QString &vehicle_id) {
    this->vehicle_id = vehicle_id;
    this->m_vehicle_id_isSet = true;
}

bool OAIInspectionRequest::is_vehicle_id_Set() const{
    return m_vehicle_id_isSet;
}

bool OAIInspectionRequest::is_vehicle_id_Valid() const{
    return m_vehicle_id_isValid;
}

QString OAIInspectionRequest::getContainerId() const {
    return container_id;
}
void OAIInspectionRequest::setContainerId(const QString &container_id) {
    this->container_id = container_id;
    this->m_container_id_isSet = true;
}

bool OAIInspectionRequest::is_container_id_Set() const{
    return m_container_id_isSet;
}

bool OAIInspectionRequest::is_container_id_Valid() const{
    return m_container_id_isValid;
}

QString OAIInspectionRequest::getActionId() const {
    return action_id;
}
void OAIInspectionRequest::setActionId(const QString &action_id) {
    this->action_id = action_id;
    this->m_action_id_isSet = true;
}

bool OAIInspectionRequest::is_action_id_Set() const{
    return m_action_id_isSet;
}

bool OAIInspectionRequest::is_action_id_Valid() const{
    return m_action_id_isValid;
}

bool OAIInspectionRequest::isSet() const {
    bool isObjectUpdated = false;
    do {
        if (m_vehicle_id_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_container_id_isSet) {
            isObjectUpdated = true;
            break;
        }

        if (m_action_id_isSet) {
            isObjectUpdated = true;
            break;
        }
    } while (false);
    return isObjectUpdated;
}

bool OAIInspectionRequest::isValid() const {
    // only required properties are required for the object to be considered valid
    return m_vehicle_id_isValid && m_container_id_isValid && m_action_id_isValid && true;
}

} // namespace OpenAPI