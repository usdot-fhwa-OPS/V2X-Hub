/**
 * Pedestrian Plugin API
 * No description provided (generated by Openapi Generator https://github.com/openapitools/openapi-generator)
 *
 * The version of the OpenAPI document: 1.2.0
 *
 * NOTE: This class is auto generated by OpenAPI Generator (https://openapi-generator.tech).
 * https://openapi-generator.tech
 * Do not edit the class manually.
 */

/*
 * OAIPersonalSafetyMessage_accuracy.h
 *
 * 
 */

#ifndef OAIPersonalSafetyMessage_accuracy_H
#define OAIPersonalSafetyMessage_accuracy_H

#include <QJsonObject>


#include "OAIEnum.h"
#include "OAIObject.h"

namespace OpenAPI {

class OAIPersonalSafetyMessage_accuracy : public OAIObject {
public:
    OAIPersonalSafetyMessage_accuracy();
    OAIPersonalSafetyMessage_accuracy(QString json);
    ~OAIPersonalSafetyMessage_accuracy() override;

    QString asJson() const override;
    QJsonObject asJsonObject() const override;
    void fromJsonObject(QJsonObject json) override;
    void fromJson(QString jsonString) override;

    double getSemiMajor() const;
    void setSemiMajor(const double &semi_major);
    bool is_semi_major_Set() const;
    bool is_semi_major_Valid() const;

    double getSemiMinor() const;
    void setSemiMinor(const double &semi_minor);
    bool is_semi_minor_Set() const;
    bool is_semi_minor_Valid() const;

    double getOrientation() const;
    void setOrientation(const double &orientation);
    bool is_orientation_Set() const;
    bool is_orientation_Valid() const;

    virtual bool isSet() const override;
    virtual bool isValid() const override;

private:
    void initializeModel();

    double semi_major;
    bool m_semi_major_isSet;
    bool m_semi_major_isValid;

    double semi_minor;
    bool m_semi_minor_isSet;
    bool m_semi_minor_isValid;

    double orientation;
    bool m_orientation_isSet;
    bool m_orientation_isValid;
};

} // namespace OpenAPI

Q_DECLARE_METATYPE(OpenAPI::OAIPersonalSafetyMessage_accuracy)

#endif // OAIPersonalSafetyMessage_accuracy_H