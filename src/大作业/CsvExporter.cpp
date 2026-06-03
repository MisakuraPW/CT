#include "pch.h"
#include "CsvExporter.h"

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>

#include <atlconv.h>

namespace
{
    std::string Narrow(const CString& value)
    {
        CT2A converted(value, CP_UTF8);
        return std::string(converted);
    }

    std::ofstream OpenUtf8Csv(const std::wstring& path)
    {
        std::ofstream output(path, std::ios::binary);
        if (output)
        {
            const unsigned char bom[] = { 0xEF, 0xBB, 0xBF };
            output.write(reinterpret_cast<const char*>(bom), sizeof(bom));
        }
        return output;
    }
}

namespace CsvExporter
{
    bool ExportMetrics(const std::wstring& path, const CString& filename, const SegmentationMetrics& metrics)
    {
        std::ofstream output = OpenUtf8Csv(path);
        if (!output)
        {
            return false;
        }

        output << "filename,tp,fp,tn,fn,dice,iou,precision,recall,area_error\n";
        output << '"' << Narrow(filename) << '"' << ','
            << metrics.tp << ','
            << metrics.fp << ','
            << metrics.tn << ','
            << metrics.fn << ','
            << std::fixed << std::setprecision(6)
            << metrics.dice << ','
            << metrics.iou << ','
            << metrics.precision << ','
            << metrics.recall << ','
            << metrics.areaError << '\n';

        return output.good();
    }

    bool ExportInfectionStats(const std::wstring& path, const CString& filename, const InfectionStats& stats)
    {
        std::ofstream output = OpenUtf8Csv(path);
        if (!output)
        {
            return false;
        }

        output << "filename,lung_area,infection_area,infection_ratio,left_lung_area,left_infection_area,left_ratio,right_lung_area,right_infection_area,right_ratio\n";
        output << '"' << Narrow(filename) << '"' << ','
            << stats.lungArea << ','
            << stats.infectionArea << ','
            << std::fixed << std::setprecision(6)
            << stats.infectionRatio << ','
            << stats.leftLungArea << ','
            << stats.leftInfectionArea << ','
            << stats.leftRatio << ','
            << stats.rightLungArea << ','
            << stats.rightInfectionArea << ','
            << stats.rightRatio << '\n';

        return output.good();
    }
}
