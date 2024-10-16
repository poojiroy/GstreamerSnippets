#include <stdio.h>
#include <gst/gst.h>
static void on_pad_added(GstElement *src, GstPad *new_pad, GstElement *converter){
    GstPad *converter_pad = gst_element_get_static_pad(converter, "sink");
    if(gst_pad_link(new_pad, converter_pad)!= GST_PAD_LINK_OK){
        g_printerr("Failed to link");
        
    }
    gst_object_unref(converter_pad);
}
int main(int argc, char *argv[]){
    GstElement *pipeline, *source, *decoder, *converter, *scale, *filter, *sink;
    GstBus *bus;
    GstMessage *msg;
    gst_init(&argc,&argv);
    pipeline=gst_pipeline_new("scaling");
    source = gst_element_factory_make("filesrc", "source");
    g_object_set(source, "location", "1080pQuality.mp4", NULL);
    decoder = gst_element_factory_make("decodebin", "decoder");
    converter = gst_element_factory_make("videoconvert", "converter");
    scale= gst_element_factory_make("videoscale","scale");
    filter = gst_element_factory_make("capsfilter", "filter");
    GstCaps *caps = gst_caps_new_simple("video/x-raw",
                                        "width", G_TYPE_INT, 640,
                                        "height", G_TYPE_INT,480, NULL    );
    g_object_set(filter, "caps", caps, NULL);
    gst_caps_unref(caps);                                    
    sink=gst_element_factory_make("autovideosink", "sink");
    if(!pipeline || !source || !decoder || !converter || !scale || !filter || !sink){
        g_printerr("Elements are missing ");
        return -1;
    }
    gst_bin_add_many(GST_BIN(pipeline), source, decoder, converter, scale, filter, sink, NULL);
    gst_element_link(source, decoder);
    g_signal_connect(decoder, "pad-added", G_CALLBACK(on_pad_added), converter);
    gst_element_link(converter, scale);
    gst_element_link(scale, filter);
    gst_element_link(filter, sink);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
    bus=gst_element_get_bus(pipeline);
    msg=gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
    switch (GST_MESSAGE_TYPE(msg))
    {
    case GST_MESSAGE_ERROR:
    GError *err;
    gchar *debug_info;
    gst_message_parse_error(msg, &err, &debug_info);
    g_printerr("Error Info %s:%s\n", GST_OBJECT_NAME(msg->src), err->message);
    g_print("Debug Info %s\n", debug_info?debug_info:"none");
    g_error_free(err);
    g_free(debug_info);
        break;
    case GST_MESSAGE_EOS:
    g_print("End of the stream Reached\n");
    break;    
    
    default:
    g_printerr("unexpected Error\n");
        break;
    }
    gst_message_unref(msg);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);
    return 0;
}