#include <gst/gst.h>
#include <stdio.h>

static void pad_handed_handler(GstElement *src, GstPad *new_pad, GstElement *sink) {
    GstPad *sink_pad = gst_element_get_static_pad(sink, "sink"); // Correctly referencing the pad name
    if (gst_pad_is_linked(sink_pad)) {
        g_print("Sink pad is already linked\n");
    } else {
        GstPadLinkReturn ret = gst_pad_link(new_pad, sink_pad);
        if (GST_PAD_LINK_FAILED(ret)) {
            g_printerr("Failed to link pads\n");
        } else {
            g_print("Pads linked successfully\n");
        }
    }
    gst_object_unref(sink_pad);
}

int main(int argc, char *argv[]) {
    GstElement *pipeline, *source, *decoder, *sink;
    GstBus *bus;
    GstMessage *msg;

    gst_init(&argc, &argv);

    // Create the pipeline and elements
    pipeline = gst_pipeline_new("player");
    source = gst_element_factory_make("filesrc", "source");
    g_object_set(source, "location", "1080pQuality.mp4", NULL);
    decoder = gst_element_factory_make("decodebin", "decoder");
    sink = gst_element_factory_make("autovideosink", "sink");

    // Check if elements are created successfully
    if (!pipeline || !source || !decoder || !sink) {
        g_printerr("Elements couldn't be created\n");
        return -1;
    }

    // Add elements to the pipeline
    gst_bin_add_many(GST_BIN(pipeline), source, decoder, sink, NULL);

    // Link source to decoder
    if (!gst_element_link(source, decoder)) {
        g_printerr("Elements couldn't be linked\n");
        gst_object_unref(pipeline);
        return -1;
    }

    // Connect the pad-added signal for the decoder
    g_signal_connect(decoder, "pad-added", G_CALLBACK(pad_handed_handler), sink);

    // Set the pipeline to the playing state
    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    // Wait for error or end-of-stream messages
    bus = gst_element_get_bus(pipeline);
    msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    // Handle messages from the bus
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *err;
            gchar *debug_info;
            gst_message_parse_error(msg, &err, &debug_info);
            g_printerr("Error from element %s: %s\n", GST_OBJECT_NAME(msg->src), err->message);
            g_printerr("Debug info: %s\n", debug_info ? debug_info : "none");
            g_error_free(err);
            g_free(debug_info);
            break;
        }
        case GST_MESSAGE_EOS:
            g_print("Stream Ended\n");
            break;
        default:
            g_printerr("Unexpected message received\n");
            break;
    }

    // Clean up
    gst_message_unref(msg);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    gst_object_unref(bus);

    return 0;
}
