/**
 * @file MessageHandler.h
 * @brief Message handler for processing e-bike data from UDP
 * @date April 2025
 */

 #ifndef MESSAGE_HANDLER_H
 #define MESSAGE_HANDLER_H
 
 #include <Poco/JSON/Parser.h>
 #include <Poco/JSON/Object.h>
 #include <Poco/JSON/Array.h>
 #include <Poco/Dynamic/Var.h>
 #include <string>
 #include <iostream>
 #include <chrono>
 #include <ctime>
 #include <sstream>
 #include <iomanip>
 
 /**
  * @class MessageHandler
  * @brief Handles incoming messages from eBike clients and converts them to GeoJSON
  * 
  * This class is responsible for parsing JSON messages from eBike clients,
  * converting them to GeoJSON format, and adding them to the shared ebikes array.
  */
 class MessageHandler {
 public:
     /**
      * @brief Constructor for MessageHandler
      * @param ebikes Reference to the shared array of e-bikes in GeoJSON format
      */
     MessageHandler(Poco::JSON::Array::Ptr& ebikes) : _ebikes(ebikes) {}
 
     /**
      * @brief Get the current time as a formatted string
      * @return String with current timestamp in ISO format
      */
     std::string getCurrentTimeISO() {
         auto now = std::chrono::system_clock::now();
         std::time_t time = std::chrono::system_clock::to_time_t(now);
         
         std::stringstream ss;
         ss << std::put_time(std::gmtime(&time), "%Y-%m-%dT%H:%M:%SZ");
         return ss.str();
     }
 
     /**
      * @brief Handle incoming message from eBike client
      * @param message The message received from the client
      * @param sourceIp The source IP address of the client
      * @param sourcePort The source port of the client
      * @return Response message to send back to the client
      */
     std::string handleMessage(const std::string& message, const std::string& sourceIp, int sourcePort) {
         try {
             // Parse the JSON message
             Poco::JSON::Parser parser;
             Poco::Dynamic::Var result = parser.parse(message);
             Poco::JSON::Object::Ptr json = result.extract<Poco::JSON::Object::Ptr>();
             
             // Extract the data
             int ebikeId = json->getValue<int>("ebike_id");
             std::string timestamp = json->getValue<std::string>("timestamp");
             Poco::JSON::Object::Ptr gpsData = json->getObject("gps");
             double latitude = gpsData->getValue<double>("latitude");
             double longitude = gpsData->getValue<double>("longitude");
             
             // Create GeoJSON feature
             Poco::JSON::Object::Ptr geoJson = new Poco::JSON::Object;
             geoJson->set("type", "Feature");
             
             // Set geometry
             Poco::JSON::Object::Ptr geometry = new Poco::JSON::Object;
             geometry->set("type", "Point");
             Poco::JSON::Array::Ptr coordinates = new Poco::JSON::Array;
             coordinates->add(longitude); // GeoJSON uses [lon, lat] order
             coordinates->add(latitude);
             geometry->set("coordinates", coordinates);
             geoJson->set("geometry", geometry);
             
             // Set properties
             Poco::JSON::Object::Ptr properties = new Poco::JSON::Object;
             properties->set("id", ebikeId);
             properties->set("timestamp", timestamp);
             properties->set("status", "unlocked"); // Default status
             geoJson->set("properties", properties);
             
             // Check if this ebike already exists in the array
             bool found = false;
             for (size_t i = 0; i < _ebikes->size(); i++) {
                 Poco::JSON::Object::Ptr existingFeature = _ebikes->getObject(i);
                 Poco::JSON::Object::Ptr existingProps = existingFeature->getObject("properties");
                 if (existingProps->getValue<int>("id") == ebikeId) {
                     // Update existing feature
                     _ebikes->set(i, geoJson);
                     found = true;
                     break;
                 }
             }
             
             // If not found, add as new feature
             if (!found) {
                 _ebikes->add(geoJson);
             }
             
             std::cout << "Received data from eBike " << ebikeId 
                       << " at " << latitude << ", " << longitude 
                       << " from " << sourceIp << ":" << sourcePort << std::endl;
             
             // Return acknowledgment
             return "OK";
         } catch (const std::exception& e) {
             std::cerr << "Error handling message: " << e.what() << std::endl;
             std::cerr << "Message content: " << message << std::endl;
             return "ERROR: " + std::string(e.what());
         }
     }
 
 private:
     Poco::JSON::Array::Ptr& _ebikes; ///< Reference to the shared ebikes array
 };
 
 #endif // MESSAGE_HANDLER_Hs