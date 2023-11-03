# Field Trials


## Installation

There are a number of dependencies for these services. You need the [grassroots core](https://github.com/TGAC/grassroots-core) and [grassroots build config](https://github.com/TGAC/grassroots-build-config) installed and configured. Alongside these you need to install the [frictionless data](https://github.com/TGAC/grassroots-frictionless-data) and [geocoder](https://github.com/TGAC/grassroots-geocoder) libraries. 


The field trials services also use the [libexif](https://github.com/libexif/libexif) library which
we previously installed with the `install_dependencies` script that we ran previously. We need
to let this service know the folder where we installed libexif. This is done by creating a 
properties file which specifies this. An example file is part of the field trials directory 
structure `build/unix/linux/example_user.prefs`. We copy this to a file called `linux/user.prefs` 
which we will then edit to specify the location of libexif.


```
cp build/unix/example_user.prefs build/unix/linux/user.prefs
```

The content of this file is shown below

```
#
# field trial dependencies
#
# Set this to where you have the libexif directory 
# containing "include" and "lib" subdirectories.
export LIBEXIF_HOME := /opt/libexif
```

and we need to change this to point where libexif is installed. For example, 
if it is installed at
`/opt/grassroots/extras/linexif` by changing the variable to 

```
export LIBEXIF_HOME := /opt/grassroots/extras/linexif
```

with this in place, the field trials services can be built correctly. We also need to make sure 
that the libexif libraries are in the library path when we run httpd, so we need to amend the
`bin/envvars_grassroots` file within the Apache installation that is being used to run Grassroots 
and add the appropriate line

```
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:<WHERE YOU HAVE LIBEXIF INSTALLED>/libexif/lib
```

prior to the 
```
export LD_LIBRARY_PATH
```

which is at the bottom of the file.


The files to build the Field Trials services are in the `build` directory. 

### Linux and Mac

Enter the build directory for your your given platform which is inside the `build/unix/<platform>` 

For example, under linux:

```
cd build/unix/linux
```

whereas on MacOS:

```
cd build/unix/mac
```

then

```
make all
```

and then 

```
make install
```

to install the library into the Grassroots system where it will be available for use immediately.


### Windows

Under Windows, there is a Visual Studio project in the `build/windows` folder that allows you to build the geocoder library.


## Services and configuration

There are a number of services as part of the Field Trials service module and these are detailed 
below. As well as their individual configuration options, they all share the following keys:


 * **so:image**: The web address of the image to use for this service's logo
 * **database**: The name of the database that the service should query. 
This database should be set to the same value for all of the field trial services described below.


We will now describe each of the individual services and their configuration options.



### Submit Field Trial Programme	

Add a Programme to the system. A Programme contains one or more Trials.	
It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/submit_programme.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.

These are:

*	**cache_path**: When generating the programmes, you can choose that rather than generate these files each time, they can be generated after each submission or edit of that programme and then stored for future access.
This key specifies the directory to use to store these files
*	**view_programme_url**: When a programme has been submitted or edited, it can be useful to give the submitter a direct link to view the programme in the field trials portal. 
The system will automatically append the programme's unique identifier to this value and present the resultant web address to the user.


### Submit Field Trials

Add a Field Trial to the system. 
Following the same nomenclature as BrAPI, a Field Trial contains multiple Studies. 
This is equivalent to an Investigation in MIAPPE. 
It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/submit_trial.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.

These are:

*	**cache_path**: When generating the programmes, you can choose that rather than generate these files each time, they can be generated after each submission or edit of that programme and then stored for future access.
This key specifies the directory to use to store these files
*	**view_trial_url**: When a trial has been submitted or edited, it can be useful to give the submitter a direct link to view the trial in the field trials portal. 
The system will automatically append the trial's unique identifier to this value and present the resultant web address to the user.



### Submit Field Trial Study	

Following the same nomenclature as BrAPI, a Study is a phenotyping experiment taking place at a single location. 
One or more Studies can take place within a single Trial. 
It contains substantial amounts of metadata and these are described in the [user](https://grassroots.tools/docs/user/services/field_trial/submit_study.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.
These are:

*	**cache_path**: When generating the studies, the resultant files can potentially be very large if there are many plots or gps data. 
Rather than generate these files each time, with the resultant time that they take, when a given study is requested, these can be generated after each submission or edit of that study and then stored for future access.
This key specifies the directory to use to store these files
* **fd_path**: Grassroots can generate both [Frictionless Data Packages](https://frictionlessdata.io/) and PDF handbooks for each Study. 
This is the filesystem path to where these files will be created.
* **fd_url**: This is the web address to the path specified by the `fd_path` key detailed above.
*	**view_study_url**: When a study has been submitted or edited, it can be useful to give the submitter a direct link to view the study in the field trials portal. 
The system will automatically append the study's unique identifier to this value and present the resultant web address to the user.

* **pdflatex_path**: Grassroots uses PDFLatex to create the study handbooks. This value is the path to the `pdflatex` program that will be used to generate them.

* **geoapify_api_key**: [Geoapify](https://www.geoapify.com/) is a web platform to create maps, route optimization, and location intelligence apps. 
When generating the handbook files, Grassroots will attempt to insert a map tile graphic of the study's location into the handbook. 
It does this by calling Geoapigy's REST web services. 
To do this you need to have an API key from them and it's this value that is added here.

*	**handbook_phenotype_images**: Part of the [Django-based Grassroots front-end](../servers/django) server is its ability to generate heatmaps for each set of phenotypic observations within a study. 
These can then be inserted When generating the handbooks.
This value is the filesystem path to where they are generated. For more information, refer to the Django server guide.



### Submit Field Trial Plots	

This is a service to submit field trial plots for a given study. 
It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/submit_plots.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.
These are:

*	**cache_path**: When generating the studies, the resultant files can potentially be very large if there are many plots or gps data. 
Rather than generate these files each time, with the resultant time that they take, when a given study is requested, these can be generated after each submission or edit of that study and then stored for future access.
This key specifies the directory to use to store these files
* **fd_path**: Grassroots can generate both [Frictionless Data Packages](https://frictionlessdata.io/) and PDF handbooks for each Study. 
This is the filesystem path to where these files will be created.
* **fd_url**: This is the web address to the path specified by the `fd_path` key detailed above.
*	**view_plots_url**: When some plots have been submitted or edited, it can be useful to give the submitter a direct link to view the new set of plots in the field trials portal. 
The system will automatically append the study's unique identifier to this value and present the resultant web address to the user.



### Edit Field Trial Rack	

A service to edit an individual field trial plot or rack.
This is useful if you wish to edit just a single plot rather than building a spreadsheet.
For example, if you are walking through a field experiment or greenhouse and measuring values at each plot/rack in turn.

It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/edit_rack.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.
These are:

*	**cache_path**: When generating the studies, the resultant files can potentially be very large if there are many plots or gps data. 
Rather than generate these files each time, with the resultant time that they take, when a given study is requested, these can be generated after each submission or edit of that study and then stored for future access.
This key specifies the directory to use to store these files
* **fd_path**: Grassroots can generate both [Frictionless Data Packages](https://frictionlessdata.io/) and PDF handbooks for each Study. 
This is the filesystem path to where these files will be created.
* **fd_url**: This is the web address to the path specified by the `fd_path` key detailed above.
*	**view_plots_url**: When some plots have been submitted or edited, it can be useful to give the submitter a direct link to view the new set of plots in the field trials portal. 
The system will automatically append the study's unique identifier to this value and present the resultant web address to the user.


### Submit Field Trial Location

This is a service to define crops that are available within the system. 
It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/submit_location.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it has a number of additional parameters.
These are:

*	**view_location_url**: When a location has been submitted or edited, it can be useful to give the submitter a direct link to view the location, along with any studies performed there, in the field trials portal. 
The system will automatically append the trial's unique identifier to this value and present the resultant web address to the user.


### Submit Field Trial Measured Variables	

This is a service to submit field trial measured variables.
For each Study, you can specify the set of phenotypes that will be measured and these are called *Measured Phenotype Variables*.
Each of these consist of unique triples (three distinct pieces of information) that define:

* Trait: This is a phenotype *e.g.* plant height.
* Method: How the phenotype has been measured *e.g.*
from ground to top of spike, excluding awns.
* Unit: Which units have been used *e.g.* cm.

These are added to Grassroots by compiling a spreadsheet with the
relevant information.
The only configuration keys that this service has are the common ones of `so:image` and `database`.


### Submit Field Trial Crop	

This is a service to define crops that are available within the system. 
The only configuration keys that it has are the common ones of `so:image` and `database`.


### Submit Field Trial Gene Banks	

A service to submit field trial gene banks. 
These are seed banks that store and allow purchase of seeds for different 
species and varieties of crops.
The only configuration keys that this service has are the common ones of `so:image` and `database`.


### Search Field Trials	

This is a service to search field trial data.
It is described in the [user](https://grassroots.tools/docs/user/services/field_trial/search_portal.md) guide for this service.
As well as the common `so:image` and 
`database` configuration options, it also has the following parameters.

* **images**: There are the following facet types defined for the different classes of field trial data. 

	* Grassroots:FieldTrial
	* Grassroots:Location
	* Grassroots:Phenotype
	* Grassroots:Programme
	* Grassroots:Study
	* Grassroots:Treatment

For each of these, you can specify the icon image used for any hits of these types within a set of search results. For example:

```json
	"images": {
		"Grassroots:FieldTrial": "http://localhost:images/trial.jpeg",
		"Grassroots:Location": "http://localhost:images/location.jpeg",
		"Grassroots:Phenotype": "http://localhost:images/phenotype.jpeg",
		"Grassroots:Programme": "http://localhost:images/programme.jpeg",
		"Grassroots:Study": "http://localhost:images/study.jpeg",
		"Grassroots:Treatment": "http://localhost:images/treatment.jpeg"
	}
```

### Submit Field Trial Treatments	

This is a service to add Treatments for a given Study.
Although this service can be used, there is a section for adding treatments and treatment 
factors within the *Submit Field Trial Study* service which is the preferred method of doing this.
As well as the common `so:image` and 
`database` configuration options, it has the following additional parameters:

*	**cache_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.


### Submit Field Trial Treatment Factor	

This is a service to add Treatment Factors for one of a given study's treatments.
Although this service can be used, there is a section for adding treatments and treatment 
factors within the *Submit Field Trial Study* service which is the preferred method of doing this.
As well as the common `so:image` and 
`database` configuration options, it has the following additional parameters:

*	**cache_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.


### Manage Study	

This is a service that allows you to perform various administrative tasks for a given study such as clearing its cache, regenerating its PDF handbook, *etc.* 

As well as the common `so:image` and 
`database` configuration options, it has the following additional parameters:

*	**cache_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **fd_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **fd_url**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **view_study_url**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **pdflatex_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **geoapify_api_key**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **handbook_phenotype_images**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.


### Manage Field Trial data	

This is a service that allows you to perform various administrative tasks for the different types of data within the system.  

As well as the common `so:image` and 
`database` configuration options, it has the following additional parameters:

*	**cache_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **fd_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **fd_url**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **view_study_url**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **pdflatex_path**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **geoapify_api_key**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.

* **handbook_phenotype_images**: This should be the same as the value set for the *Submit Field Trial Study* service and is described there.



### Browse Field Trial Revisions	

Within Grassroots, each piece of Field Trial data can be potentially edited multiple times. 
There can often be occasions when you would like to view previous revisions of these items and this service allows you to do that for field trials.

As well as the common `so:image` and 
`database` configuration options, it has the following additional parameters:

*	**view_trial_url**: This is the same as the *view_trial_url* within the *Submit Field Trials* service and is described there.
