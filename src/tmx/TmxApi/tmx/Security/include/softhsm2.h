

#include <string>
#include <vector>
#include <memory>
#include <numeric>

#include <botan/p11.h>
#include <botan/p11_object.h>
#include <botan/p11_randomgenerator.h>
#include <botan/pubkey.h>
#include <botan/rsa.h>
#include <botan/p11_rsa.h>

#include <botan/ecdsa.h>
#include <botan/p11_ecdsa.h>


using namespace Botan;
using namespace PKCS11;

class softhsm
   {
   public:
      softhsm():
      m_module(new Module("/usr/local/lib/softhsm/libsofthsm2.so"))
         {
            bool login=false; 
            PIN="1234";
            ec_dompar_encoding = Botan::EC_Group_Encoding:: EC_DOMPAR_ENC_OID;
            std::vector<SlotId> slot_vec = Slot::get_available_slots(*m_module, false);
            m_slot.reset(new Slot(*m_module, slot_vec.at(0)));
            m_session.reset(new Session(*m_slot, false));
         }
      inline bool loginSession()
      {
         m_session->login(UserType::User, Botan::PKCS11::secure_string(PIN.begin(), PIN.end()));
      }
      inline Session& session() const
         {
         return *m_session;
         }

      inline Slot& slot() const
         {
         return *m_slot;
         }

      inline bool get_info()
      {
          
	         Botan::PKCS11::Info info = m_module->get_info();


           printf("%s . %s\n",std::to_string( info.libraryVersion.major), std::to_string( info.libraryVersion.minor));  
      }
      inline bool get_available_slots() 
      {
         std::vector<SlotId> slot_vec = Slot::get_available_slots(*m_module, false);
         printf("Available slots count = %d\n",slot_vec.size());
      }  


   inline PKCS11_RSA_KeyPair generate_rsa_keypair()
   {
      RSA_PublicKeyGenerationProperties pub_props(2048UL);
      pub_props.set_pub_exponent();
      pub_props.set_label("V2X_HUB_PUB_KEY_RSA");
      pub_props.set_token(true);
      pub_props.set_encrypt(true);
      pub_props.set_verify(true);
      pub_props.set_private(false);

      RSA_PrivateKeyGenerationProperties priv_props;
      priv_props.set_label("V2X_HUB_PRIV_KEY_RSA");
      priv_props.set_token(true);
      priv_props.set_private(true);
      priv_props.set_sign(true);
      priv_props.set_decrypt(true);

      return PKCS11::generate_rsa_keypair(this->session(), pub_props, priv_props);
   }

   inline int encrypt_rsa()
   {


   // generate key pair
      PKCS11_RSA_KeyPair keypair = generate_rsa_keypair();

      auto encrypt_and_decrypt = [&keypair](const std::vector<uint8_t>& plaintext, const std::string& padding)
      {
      Botan::PK_Encryptor_EME encryptor(keypair.first, randomgen, padding);
      auto encrypted = encryptor.encrypt(plaintext, randomgen);

      //Botan::PK_Decryptor_EME decryptor(keypair.second, Test::rng(), padding);
      //auto decrypted = decryptor.decrypt(encrypted);
      };

   }
   inline int decrypt_rsa()
   {


   // generate key pair
      PKCS11_RSA_KeyPair keypair = generate_rsa_keypair();

      auto encrypt_and_decrypt = [&keypair](const std::vector<uint8_t>& encrypted, const std::string& padding)
      {

         Botan::PK_Decryptor_EME decryptor(keypair.second, randomgen, padding);
         auto decrypted = decryptor.decrypt(encrypted);
      };

   }
   private:
      std::unique_ptr<Module> m_module = nullptr;
      std::unique_ptr<Slot> m_slot = nullptr;
      std::unique_ptr<Session> m_session = nullptr;
      std::string PIN; 
      static std::unique_ptr<Botan::RandomNumberGenerator> randomgen;

   };
std::unique_ptr<Botan::RandomNumberGenerator> softhsm::randomgen;
