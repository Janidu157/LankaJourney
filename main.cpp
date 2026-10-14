#include <algorithm>
#include <cctype>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

class JsonValue {
public:
    enum class Kind { Null, Boolean, Number, String, Array, Object };

    Kind kind = Kind::Null;
    bool boolean = false;
    double number = 0;
    std::string string;
    std::vector<JsonValue> array;
    std::map<std::string, JsonValue> object;

    const JsonValue& get(const std::string& key) const {
        static const JsonValue missing;
        const auto found = object.find(key);
        return found == object.end() ? missing : found->second;
    }

    std::string text() const {
        if (kind == Kind::String) return string;
        if (kind == Kind::Number) return std::to_string(number);
        if (kind == Kind::Boolean) return boolean ? "true" : "false";
        return "";
    }

    double numeric(double fallback = 0) const {
        return kind == Kind::Number ? number : fallback;
    }
};

class JsonParser {
public:
    explicit JsonParser(const std::string& input) : input_(input) {}

    JsonValue parse() {
        JsonValue result = value();
        whitespace();
        if (position_ != input_.size()) throw std::runtime_error("Unexpected data after JSON input.");
        return result;
    }

private:
    const std::string& input_;
    size_t position_ = 0;

    void whitespace() {
        while (position_ < input_.size() && std::isspace(static_cast<unsigned char>(input_[position_]))) ++position_;
    }

    char take() {
        if (position_ >= input_.size()) throw std::runtime_error("Unexpected end of JSON input.");
        return input_[position_++];
    }

    bool consume(char expected) {
        whitespace();
        if (position_ < input_.size() && input_[position_] == expected) {
            ++position_;
            return true;
        }
        return false;
    }

    void expect(char expected) {
        if (!consume(expected)) throw std::runtime_error("Invalid JSON input.");
    }

    static void appendUtf8(std::string& output, unsigned int codePoint) {
        if (codePoint <= 0x7F) {
            output.push_back(static_cast<char>(codePoint));
        } else if (codePoint <= 0x7FF) {
            output.push_back(static_cast<char>(0xC0 | (codePoint >> 6)));
            output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else if (codePoint <= 0xFFFF) {
            output.push_back(static_cast<char>(0xE0 | (codePoint >> 12)));
            output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        } else {
            output.push_back(static_cast<char>(0xF0 | (codePoint >> 18)));
            output.push_back(static_cast<char>(0x80 | ((codePoint >> 12) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | ((codePoint >> 6) & 0x3F)));
            output.push_back(static_cast<char>(0x80 | (codePoint & 0x3F)));
        }
    }

    unsigned int hexQuad() {
        unsigned int value = 0;
        for (int i = 0; i < 4; ++i) {
            const char digit = take();
            value <<= 4;
            if (digit >= '0' && digit <= '9') value |= static_cast<unsigned int>(digit - '0');
            else if (digit >= 'a' && digit <= 'f') value |= static_cast<unsigned int>(digit - 'a' + 10);
            else if (digit >= 'A' && digit <= 'F') value |= static_cast<unsigned int>(digit - 'A' + 10);
            else throw std::runtime_error("Invalid Unicode escape in JSON input.");
        }
        return value;
    }

    std::string stringValue() {
        expect('"');
        std::string result;
        while (true) {
            const char character = take();
            if (character == '"') return result;
            if (static_cast<unsigned char>(character) < 0x20) throw std::runtime_error("Invalid control character in JSON string.");
            if (character != '\\') {
                result.push_back(character);
                continue;
            }

            switch (take()) {
                case '"': result.push_back('"'); break;
                case '\\': result.push_back('\\'); break;
                case '/': result.push_back('/'); break;
                case 'b': result.push_back('\b'); break;
                case 'f': result.push_back('\f'); break;
                case 'n': result.push_back('\n'); break;
                case 'r': result.push_back('\r'); break;
                case 't': result.push_back('\t'); break;
                case 'u': {
                    unsigned int codePoint = hexQuad();
                    if (codePoint >= 0xD800 && codePoint <= 0xDBFF) {
                        if (take() != '\\' || take() != 'u') throw std::runtime_error("Invalid Unicode surrogate in JSON input.");
                        const unsigned int low = hexQuad();
                        if (low < 0xDC00 || low > 0xDFFF) throw std::runtime_error("Invalid Unicode surrogate in JSON input.");
                        codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                    } else if (codePoint >= 0xDC00 && codePoint <= 0xDFFF) {
                        throw std::runtime_error("Invalid Unicode surrogate in JSON input.");
                    }
                    appendUtf8(result, codePoint);
                    break;
                }
                default: throw std::runtime_error("Invalid escape in JSON input.");
            }
        }
    }

    JsonValue value() {
        whitespace();
        if (position_ >= input_.size()) throw std::runtime_error("Missing JSON value.");
        const char first = input_[position_];
        if (first == '"') {
            JsonValue result;
            result.kind = JsonValue::Kind::String;
            result.string = stringValue();
            return result;
        }
        if (first == '{') {
            JsonValue result;
            result.kind = JsonValue::Kind::Object;
            ++position_;
            whitespace();
            if (consume('}')) return result;
            while (true) {
                whitespace();
                if (position_ >= input_.size() || input_[position_] != '"') throw std::runtime_error("Invalid JSON object key.");
                std::string key = stringValue();
                expect(':');
                result.object.emplace(std::move(key), value());
                if (consume('}')) return result;
                expect(',');
            }
        }
        if (first == '[') {
            JsonValue result;
            result.kind = JsonValue::Kind::Array;
            ++position_;
            whitespace();
            if (consume(']')) return result;
            while (true) {
                result.array.push_back(value());
                if (consume(']')) return result;
                expect(',');
            }
        }
        if (first == 't' && input_.compare(position_, 4, "true") == 0) {
            position_ += 4;
            JsonValue result;
            result.kind = JsonValue::Kind::Boolean;
            result.boolean = true;
            return result;
        }
        if (first == 'f' && input_.compare(position_, 5, "false") == 0) {
            position_ += 5;
            JsonValue result;
            result.kind = JsonValue::Kind::Boolean;
            return result;
        }
        if (first == 'n' && input_.compare(position_, 4, "null") == 0) {
            position_ += 4;
            return JsonValue();
        }
        const size_t start = position_;
        if (input_[position_] == '-') ++position_;
        while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_;
        if (position_ < input_.size() && input_[position_] == '.') {
            ++position_;
            while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_;
        }
        if (position_ < input_.size() && (input_[position_] == 'e' || input_[position_] == 'E')) {
            ++position_;
            if (position_ < input_.size() && (input_[position_] == '+' || input_[position_] == '-')) ++position_;
            while (position_ < input_.size() && std::isdigit(static_cast<unsigned char>(input_[position_]))) ++position_;
        }
        if (start == position_) throw std::runtime_error("Invalid JSON value.");
        JsonValue result;
        result.kind = JsonValue::Kind::Number;
        result.number = std::stod(input_.substr(start, position_ - start));
        if (!std::isfinite(result.number)) throw std::runtime_error("JSON numbers must be finite.");
        return result;
    }
};

static std::string lower(std::string value) {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char character) {
        return static_cast<char>(std::tolower(character));
    });
    return value;
}

static bool contains(const std::string& text, const std::string& fragment) {
    return text.find(fragment) != std::string::npos;
}

static std::string escaped(const std::string& value) {
    std::ostringstream output;
    output << '"';
    for (unsigned char character : value) {
        switch (character) {
            case '"': output << "\\\""; break;
            case '\\': output << "\\\\"; break;
            case '\b': output << "\\b"; break;
            case '\f': output << "\\f"; break;
            case '\n': output << "\\n"; break;
            case '\r': output << "\\r"; break;
            case '\t': output << "\\t"; break;
            default:
                if (character < 0x20) {
                    output << "\\u" << std::hex << std::setw(4) << std::setfill('0') << static_cast<int>(character) << std::dec;
                } else {
                    output << static_cast<char>(character);
                }
        }
    }
    output << '"';
    return output.str();
}

static std::string jsonNumber(double value) {
    std::ostringstream output;
    output << std::setprecision(12) << value;
    return output.str();
}

struct Place {
    std::string id, name, type, experience, climate, accessibility, difficulty, town, district;
    double latitude = 0, longitude = 0;
    bool hasCoordinates = false;
};

struct Accommodation {
    std::string id, name, type, town, district;
    double price = 0, rating = 0, latitude = 0, longitude = 0;
    bool hasCoordinates = false;
};

struct Restaurant {
    std::string id, name, town, district;
    double rating = 0, latitude = 0, longitude = 0;
    bool hasCoordinates = false;
};

class user {
public:
    double accommodationBudget = 0, foodBudget = 0, transportBudget = 0, activityBudget = 0;
    double totalBudget = 0, budgetPerDay = 0;
    int days = 0, nights = 0;
    std::string startingPointName;
    double startingPointLatitude = 0, startingPointLongitude = 0;
    std::vector<std::string> destinations;

    explicit user(const JsonValue& requirements) {
        accommodationBudget = requirements.get("accommodationBudget").numeric();
        foodBudget = requirements.get("foodBudget").numeric();
        transportBudget = requirements.get("transportBudget").numeric();
        activityBudget = requirements.get("activityBudget").numeric();
        totalBudget = requirements.get("totalBudget").numeric();
        budgetPerDay = requirements.get("budgetPerDay").numeric();
        days = static_cast<int>(requirements.get("days").numeric());
        nights = static_cast<int>(requirements.get("nights").numeric());
        const JsonValue& startingPoint = requirements.get("startingPoint");
        const JsonValue& startingLatitude = startingPoint.get("latitude");
        const JsonValue& startingLongitude = startingPoint.get("longitude");
        startingPointName = startingPoint.get("name").text();
        startingPointLatitude = startingLatitude.numeric();
        startingPointLongitude = startingLongitude.numeric();
        for (const JsonValue& destination : requirements.get("destinations").array) {
            if (destination.kind == JsonValue::Kind::String) destinations.push_back(destination.string);
        }
        if (days < 1 || days > 30 || nights < 0 || nights > days || destinations.empty() ||
            startingPointName.empty() || startingLatitude.kind != JsonValue::Kind::Number ||
            startingLongitude.kind != JsonValue::Kind::Number || !std::isfinite(startingPointLatitude) ||
            startingPointLatitude < -90 || startingPointLatitude > 90 ||
            !std::isfinite(startingPointLongitude) ||
            startingPointLongitude < -180 || startingPointLongitude > 180) {
            throw std::runtime_error("Journey requirements are invalid.");
        }
    }

    double distanceFromStartingPoint(const Place& place) const {
        if (!place.hasCoordinates) return std::numeric_limits<double>::infinity();
        constexpr double pi = 3.14159265358979323846;
        const double latitude1 = startingPointLatitude * pi / 180;
        const double latitude2 = place.latitude * pi / 180;
        const double latitudeDelta = (place.latitude - startingPointLatitude) * pi / 180;
        const double longitudeDelta = (place.longitude - startingPointLongitude) * pi / 180;
        const double haversine = std::sin(latitudeDelta / 2) * std::sin(latitudeDelta / 2) +
            std::cos(latitude1) * std::cos(latitude2) *
            std::sin(longitudeDelta / 2) * std::sin(longitudeDelta / 2);
        return 6371 * 2 * std::asin(std::sqrt(std::min(1.0, haversine)));
    }

    int preferenceScore(const Place& place) const {
        const std::string details = lower(place.name + " " + place.type + " " + place.experience + " " +
                                          place.climate + " " + place.accessibility + " " + place.difficulty + " " +
                                          place.town + " " + place.district);
        int score = 0;
        for (const std::string& destination : destinations) {
            const std::string preference = lower(destination);
            if (preference == "upcountry") {
                score += contains(details, "upcountry") || contains(details, "hill") || contains(details, "mountain") ||
                                 contains(details, "highland") || contains(details, "tea") || contains(details, "ella") ||
                                 contains(details, "nuwara eliya") || contains(details, "badulla") || contains(details, "kandy")
                             ? 10 : 0;
            } else if (preference == "down south") {
                score += contains(details, "down south") || contains(details, "southern") || contains(details, "galle") ||
                                 contains(details, "matara") || contains(details, "hambantota") || contains(details, "tangalle") ||
                                 contains(details, "mirissa") || contains(details, "weligama")
                             ? 10 : 0;
            } else if (preference == "beach") {
                score += contains(details, "beach") || contains(details, "coast") || contains(details, "sea") ||
                                 contains(details, "surf") || contains(details, "galle") || contains(details, "matara")
                             ? 10 : 0;
            } else if (preference == "historical") {
                score += contains(details, "histor") || contains(details, "heritage") || contains(details, "fort") ||
                                 contains(details, "archaeolog") || contains(details, "ancient") || contains(details, "sigiriya") ||
                                 contains(details, "polonnaruwa") || contains(details, "anuradhapura")
                             ? 10 : 0;
            } else if (preference == "cultural") {
                score += contains(details, "cultur") || contains(details, "traditional") || contains(details, "heritage") ||
                                 contains(details, "temple") || contains(details, "kandy")
                             ? 10 : 0;
            } else if (preference == "religious") {
                score += contains(details, "relig") || contains(details, "temple") || contains(details, "sacred") ||
                                 contains(details, "pilgrim") || contains(details, "vihara") || contains(details, "kovil") ||
                                 contains(details, "mosque") || contains(details, "church")
                             ? 10 : 0;
            } else if (preference == "wildlife") {
                score += contains(details, "wildlife") || contains(details, "safari") || contains(details, "national park") ||
                                 contains(details, "elephant") || contains(details, "bird") || contains(details, "yala") ||
                                 contains(details, "udawalawe")
                             ? 10 : 0;
            } else if (preference == "adventure") {
                score += contains(details, "adventure") || contains(details, "hiking") || contains(details, "trek") ||
                                 contains(details, "climb") || contains(details, "rafting") || contains(details, "diving")
                             ? 10 : 0;
            } else if (preference == "shopping") {
                score += contains(details, "shopping") || contains(details, "market") || contains(details, "craft") ||
                                 contains(details, "bazaar")
                             ? 10 : 0;
            } else if (preference == "nature") {
                score += contains(details, "nature") || contains(details, "waterfall") || contains(details, "forest") ||
                                 contains(details, "lake") || contains(details, "mountain") || contains(details, "park") ||
                                 contains(details, "river") || contains(details, "scenic") || contains(details, "garden")
                             ? 10 : 0;
            }
        }
        return score;
    }

    std::string plan(const JsonValue& input) const {
        std::vector<Place> places;
        for (const JsonValue& item : input.get("places").array) {
            Place place;
            place.id = item.get("id").text();
            place.name = item.get("name").text();
            place.type = item.get("type").text();
            place.experience = item.get("experience").text();
            place.climate = item.get("climate").text();
            place.accessibility = item.get("accessibility").text();
            place.difficulty = item.get("difficulty").text();
            place.town = item.get("town").text();
            place.district = item.get("district").text();
            place.hasCoordinates = item.get("latitude").kind == JsonValue::Kind::Number &&
                                   item.get("longitude").kind == JsonValue::Kind::Number;
            if (place.hasCoordinates) {
                place.latitude = item.get("latitude").numeric();
                place.longitude = item.get("longitude").numeric();
            }
            places.push_back(std::move(place));
        }
        std::stable_sort(places.begin(), places.end(), [this](const Place& left, const Place& right) {
            const int leftScore = preferenceScore(left);
            const int rightScore = preferenceScore(right);
            if (leftScore != rightScore) return leftScore > rightScore;
            const double leftDistance = distanceFromStartingPoint(left);
            const double rightDistance = distanceFromStartingPoint(right);
            if (leftDistance != rightDistance) return leftDistance < rightDistance;
            return left.name < right.name;
        });
        places.erase(std::remove_if(places.begin(), places.end(), [this](const Place& place) {
            return preferenceScore(place) == 0;
        }), places.end());

        std::vector<Accommodation> accommodations;
        for (const JsonValue& item : input.get("accommodations").array) {
            Accommodation accommodation;
            accommodation.id = item.get("id").text();
            accommodation.name = item.get("name").text();
            accommodation.type = item.get("type").text();
            accommodation.town = item.get("town").text();
            accommodation.district = item.get("district").text();
            accommodation.price = item.get("price").numeric();
            accommodation.rating = item.get("rating").numeric();
            accommodation.hasCoordinates = item.get("latitude").kind == JsonValue::Kind::Number &&
                                           item.get("longitude").kind == JsonValue::Kind::Number;
            if (accommodation.hasCoordinates) {
                accommodation.latitude = item.get("latitude").numeric();
                accommodation.longitude = item.get("longitude").numeric();
            }
            accommodations.push_back(std::move(accommodation));
        }
        const double nightlyBudget = nights > 0 ? accommodationBudget / nights : accommodationBudget;
        const auto locationCost = [this, &places](const Accommodation& accommodation) {
            double totalDistance = 0;
            int comparisons = 0;
            const int placeCount = std::min(days, static_cast<int>(places.size()));
            for (int i = 0; i < placeCount; ++i) {
                const Place& place = places[static_cast<size_t>(i)];
                if (accommodation.hasCoordinates && place.hasCoordinates) {
                    constexpr double pi = 3.14159265358979323846;
                    const double latitude1 = accommodation.latitude * pi / 180;
                    const double latitude2 = place.latitude * pi / 180;
                    const double latitudeDelta = (place.latitude - accommodation.latitude) * pi / 180;
                    const double longitudeDelta = (place.longitude - accommodation.longitude) * pi / 180;
                    const double haversine = std::sin(latitudeDelta / 2) * std::sin(latitudeDelta / 2) +
                        std::cos(latitude1) * std::cos(latitude2) *
                        std::sin(longitudeDelta / 2) * std::sin(longitudeDelta / 2);
                    totalDistance += 6371 * 2 * std::asin(std::sqrt(std::min(1.0, haversine)));
                } else if (!accommodation.town.empty() && lower(accommodation.town) == lower(place.town)) {
                    totalDistance += 0;
                } else if (!accommodation.district.empty() && lower(accommodation.district) == lower(place.district)) {
                    totalDistance += 25;
                } else {
                    totalDistance += 1000;
                }
                ++comparisons;
            }
            return comparisons == 0 ? 0 : totalDistance / comparisons;
        };
        std::stable_sort(accommodations.begin(), accommodations.end(), [nightlyBudget, &locationCost](const Accommodation& left, const Accommodation& right) {
            const bool leftAffordable = nightlyBudget <= 0 || left.price <= nightlyBudget;
            const bool rightAffordable = nightlyBudget <= 0 || right.price <= nightlyBudget;
            if (leftAffordable != rightAffordable) return leftAffordable;
            const double leftDistance = locationCost(left);
            const double rightDistance = locationCost(right);
            if (leftDistance != rightDistance) return leftDistance < rightDistance;
            const double leftScore = left.rating * 100 - left.price / 1000;
            const double rightScore = right.rating * 100 - right.price / 1000;
            if (leftScore != rightScore) return leftScore > rightScore;
            return left.name < right.name;
        });

        std::vector<Restaurant> restaurants;
        for (const JsonValue& item : input.get("restaurants").array) {
            Restaurant restaurant;
            restaurant.id = item.get("id").text();
            restaurant.name = item.get("name").text();
            restaurant.town = item.get("town").text();
            restaurant.district = item.get("district").text();
            restaurant.rating = item.get("rating").numeric();
            restaurant.hasCoordinates = item.get("latitude").kind == JsonValue::Kind::Number &&
                                        item.get("longitude").kind == JsonValue::Kind::Number;
            if (restaurant.hasCoordinates) {
                restaurant.latitude = item.get("latitude").numeric();
                restaurant.longitude = item.get("longitude").numeric();
            }
            restaurants.push_back(std::move(restaurant));
        }

        const Accommodation* stay = accommodations.empty() ? nullptr : &accommodations.front();
        std::ostringstream output;
        output << "{\"summary\":" << escaped("A " + std::to_string(days) + "-day Sri Lanka journey planned around your selected interests.")
               << ",\"startingPoint\":{\"name\":" << escaped(startingPointName)
               << ",\"latitude\":" << jsonNumber(startingPointLatitude)
               << ",\"longitude\":" << jsonNumber(startingPointLongitude)
               << "},\"accommodationEstimate\":"
               << jsonNumber(stay == nullptr ? 0 : stay->price * nights)
               << ",\"accommodationWithinBudget\":"
               << (stay == nullptr || nightlyBudget <= 0 || stay->price <= nightlyBudget ? "true" : "false")
               << ",\"accommodation\":";
        if (stay == nullptr) {
            output << "null";
        } else {
            output << "{\"id\":" << escaped(stay->id)
                   << ",\"name\":" << escaped(stay->name)
                   << ",\"type\":" << escaped(stay->type)
                   << ",\"town\":" << escaped(stay->town)
                   << ",\"district\":" << escaped(stay->district)
                   << ",\"pricePerNight\":" << jsonNumber(stay->price)
                   << ",\"rating\":" << jsonNumber(stay->rating)
                   << ",\"latitude\":" << (stay->hasCoordinates ? jsonNumber(stay->latitude) : "null")
                   << ",\"longitude\":" << (stay->hasCoordinates ? jsonNumber(stay->longitude) : "null")
                   << '}';
        }
        output << ",\"days\":[";
        for (int day = 0; day < days; ++day) {
            if (day != 0) output << ',';
            output << "{\"day\":" << (day + 1) << ",\"destination\":";
            if (static_cast<size_t>(day) >= places.size()) {
                output << "null";
            } else {
                const Place& place = places[static_cast<size_t>(day)];
                output << "{\"id\":" << escaped(place.id)
                       << ",\"name\":" << escaped(place.name)
                       << ",\"type\":" << escaped(place.type)
                       << ",\"experience\":" << escaped(place.experience)
                       << ",\"town\":" << escaped(place.town)
                       << ",\"district\":" << escaped(place.district)
                       << ",\"latitude\":" << (place.hasCoordinates ? jsonNumber(place.latitude) : "null")
                       << ",\"longitude\":" << (place.hasCoordinates ? jsonNumber(place.longitude) : "null")
                       << '}';
            }
            const Restaurant* restaurant = nullptr;
            if (static_cast<size_t>(day) < places.size() && !restaurants.empty()) {
                const Place& place = places[static_cast<size_t>(day)];
                for (const Restaurant& candidate : restaurants) {
                    if (lower(candidate.town) == lower(place.town) ||
                        lower(candidate.district) == lower(place.district)) {
                        if (restaurant == nullptr || candidate.rating > restaurant->rating) restaurant = &candidate;
                    }
                }
            }
            output << ",\"restaurant\":";
            if (restaurant == nullptr) {
                output << "null";
            } else {
                output << "{\"id\":" << escaped(restaurant->id)
                       << ",\"name\":" << escaped(restaurant->name)
                       << ",\"town\":" << escaped(restaurant->town)
                       << ",\"district\":" << escaped(restaurant->district)
                       << ",\"rating\":" << jsonNumber(restaurant->rating)
                       << '}';
            }
            output << '}';
        }
        output << "]}";
        return output.str();
    }
};

int main() {
    try {
        std::string requirementsLine;
        if (!std::getline(std::cin, requirementsLine)) throw std::runtime_error("Journey requirements were not received.");
        const JsonValue requirements = JsonParser(requirementsLine).parse();
        user journeyUser(requirements);
        std::cout << "{\"type\":\"catalogRequest\"}" << std::endl;

        std::ostringstream catalogInput;
        catalogInput << std::cin.rdbuf();
        const JsonValue catalog = JsonParser(catalogInput.str()).parse();
        std::cout << journeyUser.plan(catalog) << std::endl;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << error.what() << std::endl;
        return 1;
    }
}
