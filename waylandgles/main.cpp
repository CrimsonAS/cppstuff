#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>
#include <wayland-client.h>
#include <wayland-egl.h>

#include <GLES/gl.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <sys/types.h>
#include <unistd.h>

class Wayland {
public:
    // Members:
    wl_display *display;
    wl_registry *registry;
    wl_compositor *compositor;
    wl_surface *surface;
    wl_egl_window *window;

    // Listeners:
    wl_registry_listener registryListener;
    static void registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version);
    static void registry_global_remove(void *data, struct wl_registry *registry, uint32_t name);

    // Initialization:
    Wayland();
};

Wayland::Wayland()
{
    display = wl_display_connect(0);
    assert(display);

    registry = wl_display_get_registry(display);
    assert(registry);

    registryListener.global = registry_global;
    registryListener.global_remove = registry_global_remove;
    wl_registry_add_listener(registry, &registryListener, this);

    wl_display_roundtrip(display);
    assert(compositor);
    
    surface = wl_compositor_create_surface(compositor);
    window = wl_egl_window_create(surface, 320, 480);

    printf("Initialized Wayland:\n");
    printf(" - display .....: %p\n", display);
    printf(" - registry ....: %p\n", registry);
    printf(" - compositor ..: %p\n", compositor);
    printf(" - surface .....: %p\n", surface);
}

void Wayland::registry_global(void *data, struct wl_registry *registry, uint32_t name, const char *interface, uint32_t version)
{
    // printf("%s: iface=%s\n", __func__, interface);
    Wayland *wl = (Wayland *) data;
    if (strcmp(interface, "wl_compositor") == 0) {
        wl->compositor = (wl_compositor *) wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    }
}

void Wayland::registry_global_remove(void *data, struct wl_registry *registry, uint32_t name)
{
    printf("%s: iface=%s\n", __func__);
}

class EGL { 
public:
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;

    EGL(Wayland *wayland);

    void makeCurrent();
    void swap();
};

EGL::EGL(Wayland *wayland)
{
    EGLint const configAttributes[] = {
        EGL_RED_SIZE, 8,
        EGL_GREEN_SIZE, 8,
        EGL_BLUE_SIZE, 8,
        EGL_ALPHA_SIZE, 8,
        EGL_NONE
    };

    EGLint const contextAttributes[] = {
        EGL_CONTEXT_CLIENT_VERSION, 2,
        EGL_NONE
    };

    EGLBoolean result;
    display = eglGetDisplay((EGLNativeDisplayType) wayland->display);
    result = eglInitialize(display, NULL, NULL);
    assert(result);

    result = eglBindAPI(EGL_OPENGL_ES_API);
    assert(result);

    int configCount;
    EGLConfig config;
    result = eglChooseConfig(display, configAttributes, &config, 1, &configCount);
    assert(result);

    surface = eglCreateWindowSurface(display, config, (EGLNativeWindowType) wayland->window, NULL);
    assert(surface);

    context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttributes);
    assert(context);

    printf("EGL Configuration:\n");
    printf(" - Display .........: %p\n", display);
    printf(" - Config ..........: %p\n", config);
    printf(" - Surface .........: %p\n", surface);
    printf(" - Context .........: %p\n", context);
    printf(" - EGL_VENDOR ......: %s\n", eglQueryString(display, EGL_VENDOR));
    printf(" - EGL_VERSION .....: %s\n", eglQueryString(display, EGL_VERSION));
    printf(" - EGL_CLIENT_APIS .: %s\n", eglQueryString(display, EGL_CLIENT_APIS));
    printf(" - EGL_EXTENSIONS ..: %s\n", eglQueryString(display, EGL_EXTENSIONS));
    int r, g, b, a, d, s;
    eglGetConfigAttrib(display, config, EGL_ALPHA_SIZE, &a);
    eglGetConfigAttrib(display, config, EGL_RED_SIZE, &r);
    eglGetConfigAttrib(display, config, EGL_GREEN_SIZE, &g);
    eglGetConfigAttrib(display, config, EGL_BLUE_SIZE, &b);
    printf(" - RGBA Buffers ....: R%d G%d B%d A%d\n", r, g, b, a);
    eglGetConfigAttrib(display, config, EGL_DEPTH_SIZE, &d);
    eglGetConfigAttrib(display, config, EGL_STENCIL_SIZE, &s);
    printf(" - Depth / Stencil .: %d / %d\n", d, s);
}

void EGL::makeCurrent()
{
    EGLBoolean ok = eglMakeCurrent(display, surface, surface, context);
    assert(ok);
}

void EGL::swap()
{
    EGLBoolean ok = eglSwapBuffers(display, surface);
    assert(ok);
}

static void checkGLError()
{
    GLenum error;
    while ((error = glGetError()) != GL_NO_ERROR) {
        printf(" -- error: %x\n", error);
    }
}

int main(int argc, char **argv)
{
    Wayland wayland;
    EGL egl(&wayland);

    wl_display_roundtrip(wayland.display);

    // Render Loop..
    int frameCount = 10;
    egl.makeCurrent();

    printf("OpenGL Configuration:\n");
    printf(" - GL_VENDOR .......: %s\n", glGetString(GL_VENDOR));
    printf(" - GL_VERSION ......: %s\n", glGetString(GL_VERSION));
    printf(" - GL_RENDERER .....: %s\n", glGetString(GL_RENDERER));
    printf(" - GL_EXTENSIONS ...: %s\n", glGetString(GL_EXTENSIONS));

    while (--frameCount) {
        int c = frameCount % 2;
        glClearColor(c, 0, 1-c, 1);


        glClear(GL_COLOR_BUFFER_BIT);

        unsigned char pixel[4];
        glReadPixels(0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

        printf("pixel: %d %d %d %d\n", 
               (int) pixel[0],
               (int) pixel[1],
               (int) pixel[2],
               (int) pixel[3]);

        egl.swap();

        wl_display_dispatch_pending(wayland.display);
    }

    return 0;
}
