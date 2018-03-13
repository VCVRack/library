## About SquinkyVCV
This project is a frequency shifter module for VCV Rack. Over time project is expected to grow from a single module to a collection of several modules.

You can find us on Facebook [here](https://www.facebook.com/SquinkyLabs)
## Manuals
Here is the user’s manual for the [frequency shifter](./docs/booty-shifter.md).
## Contributing
Please use our GitHub issues page to report problems, request features, etc. If you don’t already have a GitHub account you will need to create one, as you must be logged in to post to GitHub.

For general communications, you may use our [Facebook Page](https://www.facebook.com/SquinkyLabs)

We are not currently accepting pull requests.
## Installing binaries
Download the current release from our [releases page](https://github.com/squinkylabs/SquinkyVCV/releases).
Follow the standard instructions for installing third-party plugins in rack: [here](https://vcvrack.com/manual/Installing.html)

Note that the built-in unzip in windows explorer will create an extra folder called with the same name as the zip file. But when everything is unzipped you want your folder structure to look like this:
```
plugins/
    <other plugins>
    squinkylabs-plug1/
        plugin.dll
        plugin.dylib
        plugin.so
        LICENSE
        res/   
            <one or more svg files>
            <possibly other resources
```
    
## Building source
As with all third-party modules for VCV, you must:
* Clone the VCV Rack repo.
* Build Rack from source.
* Clone SquinkyVCV in Rack’s plugins folder.
* `CD SquinkyVCV`
* `make`

## VCV Rack version 0.6.0
The next version of VCV Rack is coming soon. Version 0.5.x plugins will not work with version 0.6.0.

We have ported our code to VCV 0.6.0. Our "master" branch is now 0.6.0dev compatible. Pulugins built from master will run in VCV Rack 0.6.0dev, and will not run in any 0.5.x versions of VCV Rack.

Please report any issues running in VCV Rack 0.6.0dev on our GitHub issues page.

Like most plugin developers, we will update our releases and post in the Plugin Manager once 0.6 ships.

