#pragma once

#include <JuceHeader.h>
#include "npy.hpp"
#include <string>
#include <cstdio>

struct Plot
{
    template <typename BufferType>
    Plot(BufferType buffer, std::string processing, std::string style, std::string label, int channel=0)
    : processing(processing)
    , style(style)
    , label(label)
    {
        //juce::File desktop = juce::File::getSpecialLocation(juce::File::userDesktopDirectory);
        //file = juce::File::createTempFile(".npy");
        //file = desktop.getChildFile("plot.npy");
        
        char* name = std::tmpnam(nullptr);
        std::cout << "temporary file name: " << name << '\n';
        
        file_name = std::string (name) + ".npy";

        const int n = buffer.length();
        std::vector<typename BufferType::value_type> vector (n);
        for (int i=0; i<n; ++i)
        {
            vector[i] = buffer.channel(channel)[i];
        }
        
        unsigned long shape[] = { (unsigned long)n };
        //npy::SaveArrayAsNumpy(file.getFullPathName().toStdString(), false, 1, shape, vector);
        npy::SaveArrayAsNumpy(file_name, false, 1, shape, vector);
    }
        
    Plot() = delete;
    
    std::string file_name;
    std::string processing;
    std::string style;
    std::string label;
};

struct Subplot
{
    bool show_legends = false;
    std::vector<Plot> plots;
};

class Figure
{

private:
    
    std::string title;
    std::vector<Subplot> subplots;
    std::vector<std::string> imports;
    
public:

    float width;
    float height;

    Figure (float w = 12, float h = 7) : width(w), height(h)
    {
        import("import numpy as np");
        import("import pylab as plt");
        import("import scipy.signal as sig");
    }
    
    void import(const std::string& s)
    {
        imports.push_back(s);
    }
    
    void subplot()
    {
        subplots.push_back(Subplot());
    }
    
    template <typename BufferType>
    void plot(BufferType buffer, int channel=0)
    {
        plot(buffer, {}, "k", {}, channel);
    }

    template <typename BufferType>
    void plot(BufferType buffer, std::string style, int channel=0)
    {
        plot(buffer, {}, style, {}, channel);
    }

    template <typename BufferType>
    void plot(BufferType buffer, std::string processing, std::string style, std::string label={}, int channel=0)
    {
        if (subplots.size() == 0)
        {
            subplot();
        }
        
        Subplot& sp = subplots[subplots.size()-1];
        
        if (label.size() != 0)
            sp.show_legends = true;
        
        sp.plots.push_back(Plot(buffer, processing, style, label, channel));
    }

    void show()
    {
        const std::string sq = "'";
        const std::string quote = "\"";
        const std::string endl = ";\n";

        std::string cmd;
    #if PURO_XCODE
        cmd = "/usr/local/bin/python3 -c \"\n";
    #elif PURO_MSVC
        cmd = "\"C:\\Python37\\python.exe\" -c \"";
    #endif
        //cmd << "import numpy as np" << endl;
        //cmd << "import pylab as plt" << endl;

        for (std::string& s : imports)
            cmd += s + endl;

        cmd += "def rtoc(r):\n";
        cmd += "  c=np.zeros(r.size//2+1,dtype='complex')" + endl;
        cmd += "  c[0]=r[0];c[-1]=r[1];c[1:-1]=r[2::2]+r[3::2]*1J" + endl;
        cmd += "  return c" + endl;
        cmd += "plt.rcParams['lines.linewidth']=1.0" + endl;

        cmd += "plt.figure(figsize=(" + std::to_string(width) + ", " + std::to_string(height) + "))" + endl;
        
        cmd += "plt.subplots_adjust(top=0.96, bottom=0.04, left=0.06, right=0.96, hspace=0.24, wspace=0.10)" + endl;
    
        int numSubPlots = (int)subplots.size();
        
        int subplotIndex = 0;

        for (auto& sp : subplots)
        {
            subplotIndex += 1;
            cmd += "plt.subplot(" + std::to_string(numSubPlots) + ", 1, " + std::to_string(subplotIndex) + ")" + endl;

            for (auto& plot : sp.plots)
            {
                if (plot.processing.size() != 0)
                {
                    std::string x = std::string("np.load('") + plot.file_name + "')";
                    
                    //std::size_t pos = plot.processing.find (plot.processing);
                    std::size_t pos = plot.processing.find ("%");
                    errorif(pos == std::string::npos, "no % found");
                    while (pos != std::string::npos)
                    {
                        plot.processing.replace(pos, 1,  x);
                        pos = plot.processing.find ("%");
                    }

                    cmd += "plt.plot( " + plot.processing + ", " + sq + plot.style + sq + ", label=" + sq + plot.label + sq + ")" + endl;
                }
                else
                {
                    std::string x = std::string("np.load(") + sq + plot.file_name + sq + ")";
                    cmd += "plt.plot( " + x + ", " + sq + plot.style + sq + ")" + endl;
                }
            }
            
            if (sp.show_legends)
                cmd += "plt.legend()" + endl;
        }
        cmd += "plt.show()" + endl;
        cmd += "\"";
        
        std::cout << "\n\n" << cmd << "\n\n";
        
        system(cmd.c_str());
    
        for (auto& sp : subplots)
            for (auto& plot : sp.plots)
                std::remove(plot.file_name.c_str());
                //plot.file.deleteFile();
    }
};

