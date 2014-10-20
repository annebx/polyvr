#ifndef OPTIONS_H_INCLUDED
#define OPTIONS_H_INCLUDED

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <iostream>

using namespace std;
namespace bpo = boost::program_options;

class VROptions {
    private:
        VROptions();
        void operator= (VROptions v);

        template <typename T>
        void addOption(T value, string name, string description = "") {
            desc.add_options() (name.c_str(), bpo::value<T>()->default_value(value, ""), description.c_str());
        }

    public:
        int argc;
        char** argv;
        bpo::options_description desc;
        bpo::variables_map vm;

        static VROptions* get();

        template <typename T>
        T getOption(string name) {
            if (vm.count(name) == 0) {
                cout << "\nERROR, " << name << " is not an option!" << endl;
                return T();
            }

            return vm[name].as<T>();
        }

        void parse(int _argc, char** _argv);
};



#endif // OPTIONS_H_INCLUDED
