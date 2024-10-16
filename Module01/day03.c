#include <gst/gst.h>
#include <stdio.h>
static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *filter){
    GstPad *filter_pad = gst_element_get_static_pad(filter, "sink");
    if(gst_pad_link(new_pad, filter_pad) != GST_PAD_LINK_OK){
        g_warning("pads didn't get linked");
    }
    gst_object_unref(filter_pad);

}
int main(int argc, char *argv[]){
    GstElement *pipeline, *source, *decoder,*filter, *sink;
    GstBus *bus;
    GstMessage *msg;
    gst_init(&argc, &argv);
    pipeline=gst_pipeline_new("edited_video");
    source=gst_element_factory_make("filesrc", "source");
    g_object_set(source, "location", "1080pQuality.mp4", NULL);
    decoder = gst_element_factory_make("decodebin", "decoder");
    filter = gst_element_factory_make("videobalance", "filter");
    g_object_set(filter, "saturation", 0.0, NULL);
    sink=gst_element_factory_make("autovideosink", "sink");
    if(!pipeline || !source || !decoder || !filter || !sink){
        g_printerr("Elements coudn't be created");
        gst_object_unref(pipeline);
        return -1;
    }
    gst_bin_add_many(GST_BIN(pipeline), source, decoder, filter, sink, NULL);
    if(!gst_element_link(source, decoder)){
        g_printerr("Element can't be linked");
        return -1;
    }
    g_signal_connect(decoder, "pad-added", G_CALLBACK(on_pad_added), filter);
    if(!gst_element_link(filter, sink)){
        g_printerr("Elements coudn't be linked");
        return -1;
    }
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bus=gst_element_get_bus(pipeline);
    msg=gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR|GST_MESSAGE_EOS);
switch (GST_MESSAGE_TYPE(msg))
{
case GST_MESSAGE_ERROR:
GError *err;
gchar *debug_info;
gst_message_parse_error(msg, &err, &debug_info);
g_printerr("Error Info %s:%s\n", GST_OBJECT_NAME(msg->src), err->message);
g_print("debug Info %s\n", debug_info?debug_info:"none" );
g_error_free(err);
g_free(debug_info);
    break;
case GST_MESSAGE_EOS:
g_print("END of the stream Reached");
break;    

default:
g_printerr("UnexpectedError");
    break;
}
gst_message_unref(msg);
gst_element_set_state(pipeline, GST_STATE_PLAYING);
gst_object_unref(pipeline);
gst_object_unref(bus);
    

    return 0;
}