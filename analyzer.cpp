#include "analyzer.h"
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <array>
#include <algorithm>

struct ZoneData {
    int count = 0;
    std::array<int, 24> hour_count{};
};

std::unordered_map<std::string, ZoneData> zone_data;

void TripAnalyzer::ingestFile(const std::string& csvPath) {
    // TODO:
    // - open file
    // - skip header
    // - skip malformed rows
    // - extract PickupZoneID and pickup hour
    // - aggregate counts
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    
    std::ifstream csv_file(csvPath);
    if (!csv_file.is_open()) {
        std::cerr << "Unable to open file: " << csvPath << std::endl;
        return;
    }
    std::string line;
    // Get the header and check for empty file, this allows skipping the header and empty file check at the same time.
    std::getline(csv_file, line);
    if(line.empty()) return;
    while (std::getline(csv_file, line)) {
    if (line.empty()) continue;
    std::string_view sv(line);
    std::string_view trip_id, pickup_zone_id, dropoff_zone_id, pickup_date_time, distance_km, fare_amount;

    int start = 0;
    int end = 0;

    end = sv.find(',', start);
    if (end == std::string_view::npos) continue;
    trip_id=sv.substr(start,end-start);
    if(trip_id.empty()) continue;
    start = end + 1;

    end = sv.find(',', start);
    if (end == std::string_view::npos) continue;
    pickup_zone_id = sv.substr(start, end - start);
    if(pickup_zone_id.empty()) continue;
    start = end + 1;

    end = sv.find(',', start);
    if (end == std::string_view::npos) continue;
    dropoff_zone_id=sv.substr(start,end-start);
    if(dropoff_zone_id.empty()) continue;
    start= end + 1;
        
    end=sv.find(',',start);
    if(end==std::string_view::npos) continue;
    pickup_date_time=sv.substr(start,end-start);
    if(pickup_date_time.empty()) continue;
    start=end + 1;
        
    end=sv.find(',',start);
    if(end==std::string_view::npos) continue;
    distance_km=sv.substr(start,end-start);
    if(distance_km.empty()) continue;
    start=end + 1;
        
    fare_amount=sv.substr(start);
    if(fare_amount.empty()) continue;
        

    if (pickup_date_time.length() != 16 || pickup_date_time[13] != ':') continue;
    if (!isdigit(pickup_date_time[11]) || !isdigit(pickup_date_time[12])) continue;

    int hour = (pickup_date_time[11]-'0')*10 + (pickup_date_time[12]-'0');
    if (hour > 23) continue;

    std::string pickup_zone_id_str = std::string(pickup_zone_id);
    ZoneData& data = zone_data[pickup_zone_id_str];
    data.count++;
    data.hour_count[hour]++;
 }
    csv_file.close();
}

bool compareZoneCount(const ZoneCount& a, const ZoneCount& b) {
    if (a.count != b.count) return a.count > b.count; // Sort by count (desc)
    return a.zone < b.zone;                           // Sort by zone (asc)
}

std::vector<ZoneCount> TripAnalyzer::topZones(int k) const {
    // TODO:
    // - sort by count desc, zone asc
    // - return first k
    std::vector<ZoneCount> zones;
    zones.reserve(zone_data.size());
    for (auto const& [zone, data_struct] : zone_data) {
        // Call the ZoneCount default constructor while aggregating the data into the ZoneCount vector.
        zones.emplace_back(ZoneCount{zone, data_struct.count});
    }
    std::sort(zones.begin(), zones.end(),compareZoneCount);
    if(zones.size() > k) zones.resize(k);
    return zones;
}

bool compareSlotCount(const SlotCount& a, const SlotCount& b) {
    if (a.count != b.count) return a.count > b.count; // Sort by count (desc)
    if (a.zone != b.zone) return a.zone < b.zone;     // Sort by zone (asc)
    return a.hour < b.hour;                           // Sort by hour (asc)
}

std::vector<SlotCount> TripAnalyzer::topBusySlots(int k) const {
    // TODO:
    // - sort by count desc, zone asc, hour asc
    // - return first k
    std::vector<SlotCount> slots;
    slots.reserve(zone_data.size()*8);
    // data_struct.hour_count[h] here is the occurrence count of a unique zone and hour pair
    for (auto const& [zone, data_struct] : zone_data)
        for (int h = 0; h < 24; h++)
            if (data_struct.hour_count[h] > 0)
                // Just inserting with emplace_back(zone, h, data_struct.hour_count[h]); doesn't work on C++17 so we had to explicitly call the default constructor here.
                slots.emplace_back(SlotCount{zone, h, data_struct.hour_count[h]});

    std::sort(slots.begin(),slots.end(),compareSlotCount);
    if(slots.size()>k) slots.resize(k);
    return slots;
}
