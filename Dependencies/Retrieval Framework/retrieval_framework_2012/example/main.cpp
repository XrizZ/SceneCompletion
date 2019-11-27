#include <QCoreApplication>
#include <QStringList>

#include <string>
#include <iostream>

#include <types.hpp>
#include <property.hpp>
#include <imagefiles.h>

using namespace imdb;
using namespace std;


int main(int argc, char *argv[])
{

    QCoreApplication app(argc, argv);


    if (app.arguments().size() != 3)
    {
        cout << "requires 2 arguments:" << endl;
        cout << "<filelist> <descriptorfile>" << endl;
        cout << "exiting" << endl;
        return EXIT_FAILURE;
    }

    // commandline parameters are path to descriptorfile
    // and path to filelist
    // Note that those two files need to be "compatible",
    // i.e. the descriptor file has been computed from the
    // filelist.
    string filelistFile = app.arguments()[1].toStdString();
    string descriptorFile = app.arguments()[2].toStdString();


    // open a reader for the descriptorfile
    // need to make sure that the template parameter
    // (vec_v32_t) corresponds to the actual datatype
    // store in the descriptorfile. This might differ
    // for each descriptor
    shared_ptr<PropertyReaderT<vec_f32_t> > reader = PropertyT<vec_f32_t>().create_reader(descriptorFile);


    // open the reader for the filelist
    ImageFiles imageFiles;
    imageFiles.load(filelistFile);
    const vector<string>& filenames = imageFiles.files();



    // go linearly over the whole descriptorfile
    // and read each single descriptor
    // Note: the ith entry in the descriptor file
    // corresponds to the ith entry in the filelist
    for (index_t i = 0; i < reader->size(); i++)
    {
        vec_f32_t feature = (*reader)[i];

        // Do something useful here, e.g. compare to
        // descriptor extracted from some input image
        // using a reasonable distance metric
        // For this example we simply output the descriptor
        // as well as the corresponding image's filename
        // to the console

        cout << "filename: " << filenames[i] << endl;
        cout << "corresponding feature: " << endl;
        for (size_t j = 0; j < feature.size(); j++)
        {
            cout << feature[j] << " ";
        }
        cout << endl;
        cout << endl;
    }


    return EXIT_SUCCESS;
}

