NAME 		:= dfw_field_trial_service
DIR_BUILD :=  $(realpath $(dir $(lastword $(MAKEFILE_LIST))))
DIR_SRC := $(realpath $(DIR_BUILD)/../../src)
DIR_INCLUDE :=$(realpath $(DIR_BUILD)/../../include)

ifeq ($(DIR_BUILD_CONFIG),)
export DIR_BUILD_CONFIG := $(realpath $(DIR_BUILD)/../../../../build-config/linux)
endif

include $(DIR_BUILD_CONFIG)/project.properties

VPATH := $(DIR_SRC)

BUILD		:= debug

export DIR_INSTALL := $(DIR_GRASSROOTS_INSTALL)/services

VPATH	= \
	$(DIR_SRC) \
	

INCLUDES = \
	-I$(DIR_INCLUDE) \
	-I$(DIR_GRASSROOTS_MONGODB_INC) \
	-I$(DIR_GRASSROOTS_SQLITE_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC) \
	-I$(DIR_GRASSROOTS_UTIL_INC)/containers \
	-I$(DIR_GRASSROOTS_UTIL_INC)/io \
	-I$(DIR_GRASSROOTS_HANDLER_INC) \
	-I$(DIR_GRASSROOTS_SERVER_INC) \
	-I$(DIR_GRASSROOTS_SERVICES_INC) \
	-I$(DIR_GRASSROOTS_NETWORK_INC) \
	-I$(DIR_GRASSROOTS_PARAMS_INC) \
	-I$(DIR_GRASSROOTS_TASK_INC) \
	-I$(DIR_GRASSROOTS_GEOCODER_INC) \
	-I$(DIR_JANSSON_INC) \
	-I$(DIR_UUID_INC) \
	-I$(DIR_MONGODB_INC) \
	-I$(DIR_BSON_INC) \
	-I$(DIR_SQLITE_INC) \
	-I$(DIR_GRASSROOTS_LUCENE_INC)
	
SRCS 	= \
	crop_ontology_tool.c \
	dfw_field_trial_service.c \
	dfw_field_trial_service_data.c \
	dfw_util.c \
	field_trial.c \
	field_trial_jobs.c \
	field_trial_mongodb.c \
	gene_bank.c \
	gene_bank_jobs.c \
	instrument.c \
	location.c \
	location_jobs.c \
	material.c \
	material_jobs.c \
	observation.c \
	phenotype.c \
	phenotype_jobs.c \
	plot.c \
	plot_jobs.c \
	row.c \
	row_jobs.c \
	search_service.c \
	study.c \
	study_jobs.c \
	submission_service.c \
	submit_field_trial.c \
	submit_gene_bank.c \
	submit_location.c \
	submit_material.c \
	submit_study.c \

CPPFLAGS += -DDFW_FIELD_TRIAL_SERVICE_EXPORTS 

LDFLAGS += -L$(DIR_JANSSON_LIB) -ljansson \
	-L$(DIR_GRASSROOTS_UTIL_LIB) -l$(GRASSROOTS_UTIL_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVICES_LIB) -l$(GRASSROOTS_SERVICES_LIB_NAME) \
	-L$(DIR_GRASSROOTS_SERVER_LIB) -l$(GRASSROOTS_SERVER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_NETWORK_LIB) -l$(GRASSROOTS_NETWORK_LIB_NAME) \
	-L$(DIR_GRASSROOTS_PARAMS_LIB) -l$(GRASSROOTS_PARAMS_LIB_NAME) \
	-L$(DIR_GRASSROOTS_MONGODB_LIB) -l$(GRASSROOTS_MONGODB_LIB_NAME)  \
	-L$(DIR_GRASSROOTS_SQLITE_LIB) -l$(GRASSROOTS_SQLITE_LIB_NAME)  \
	-L$(DIR_GRASSROOTS_GEOCODER_LIB) -l$(GRASSROOTS_GEOCODER_LIB_NAME) \
	-L$(DIR_GRASSROOTS_LUCENE_LIB) -l$(GRASSROOTS_LUCENE_LIB_NAME)  


all:: 
	make -C $(DIR_GRASSROOTS_GEOCODER_BUILD) all

include $(DIR_BUILD_CONFIG)/generic_makefiles/shared_library.makefile

