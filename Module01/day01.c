#include <stdio.h>
#include <gst/gst.h>

int main(int argc, char *argv[]){
GstElement *pipeline, *source, *sink;
GstBus *bus;
GstMessage *msg;
GstStateChangeReturn ret;
gst_init(&argc, &argv);
 pipeline =  gst_pipeline_new("test_video");
 source = gst_element_factory_make("videotestsrc", "source");
 sink = gst_element_factory_make("autovideosink", "sink");

 if(! pipeline || ! source || !sink){
    g_printerr("Elements are misssing to Create");
 }
 gst_bin_add_many(GST_BIN(pipeline), source, sink, NULL);
 if(! gst_element_link(source, sink)){
    g_printerr("Elements couldn't be linked");
    gst_object_unref(pipeline);
    return -1;
 }
 ret = gst_element_set_state(pipeline, GST_STATE_PLAYING);
 if(ret == GST_STATE_CHANGE_FAILURE){
    g_printerr("Failed to Start a pipeline");
    gst_object_unref(pipeline);
    return -1;
 }
 bus = gst_element_get_bus(pipeline);
 msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

switch (GST_MESSAGE_TYPE(msg))
{
case GST_MESSAGE_ERROR:
gchar *debug_info;
GError *err;
gst_message_parse_error(msg, &err, &debug_info);
g_printerr("Error Info %s:%s \n", GST_OBJECT_NAME(msg->src), err->message);
g_print("debug info %s\n", debug_info?debug_info: "none");
g_error_free(err);
g_free(debug_info);

    break;
case GST_MESSAGE_EOS:
g_print("Stream ended");
break;    

default:
g_print("unexpected Error");
    break;
}
gst_message_unref(msg);
gst_element_set_state(pipeline, GST_STATE_NULL);
gst_object_unref(pipeline);
gst_object_unref(bus);



    return 0;
}