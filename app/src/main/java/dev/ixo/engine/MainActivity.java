package dev.ixo.engine;

import android.app.Activity;
import android.os.Bundle;
import android.graphics.Typeface;
import android.view.Gravity;
import android.view.SurfaceHolder;
import android.view.SurfaceView;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.ScrollView;
import android.widget.TextView;
import android.widget.Toast;
import java.io.File;
import java.io.FileOutputStream;
import java.nio.charset.StandardCharsets;
import org.json.JSONArray;
import org.json.JSONObject;

public final class MainActivity extends Activity implements SurfaceHolder.Callback {
    private EditText editor;
    private TextView status;
    private File projectDir;
    private File scriptFile;
    private File sceneFile;
    private TextView sceneTree;
    private SurfaceView viewport;
    private boolean rendererStarted = false;

    private static native String nativeRendererStatus();
    private static native boolean nativeSurfaceCreated(android.view.Surface surface, android.content.res.AssetManager assets);
    private static native void nativeSurfaceChanged(int width, int height);
    private static native void nativeSurfaceDestroyed();
    static { System.loadLibrary("ixo_engine"); }

    @Override public void onCreate(Bundle state) {
        super.onCreate(state);
        getWindow().getDecorView().setSystemUiVisibility(
            View.SYSTEM_UI_FLAG_LAYOUT_STABLE | View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN);
        projectDir = new File(getFilesDir(), "IxoProject");
        File scripts = new File(projectDir, "Scripts");
        scripts.mkdirs();
        scriptFile = new File(scripts, "Player.Ixo.scripting");
        sceneFile = new File(projectDir, "Main.IxoScene");
        if (!sceneFile.exists()) saveScene(defaultScene());
        if (!scriptFile.exists()) {
            try (FileOutputStream out = new FileOutputStream(scriptFile)) {
                out.write(("-- Ixo scripting: Lua syntax\n" +
                    "function on_ready(self)\n" +
                    "    print('Hello from Ixo Engine')\n" +
                    "end\n\n" +
                    "function on_update(self, delta)\n" +
                    "    -- Update your node here\n" +
                    "end\n").getBytes(StandardCharsets.UTF_8));
            } catch (Exception ignored) { }
        }

        LinearLayout root = new LinearLayout(this);
        root.setOrientation(LinearLayout.VERTICAL);
        root.setPadding(18, 36, 18, 18);
        root.setBackgroundColor(0xFF10141E);

        TextView title = new TextView(this);
        title.setText("IXO ENGINE  /  MOBILE EDITOR");
        title.setTextColor(0xFF9CC7FF);
        title.setTypeface(Typeface.DEFAULT_BOLD);
        title.setTextSize(20);
        root.addView(title);

        status = new TextView(this);
        status.setText("Project: IxoProject\n" + nativeRendererStatus() + "\nStarting Vulkan viewport…");
        status.setTextColor(0xFFB8C5D8);
        status.setPadding(0, 10, 0, 10);
        root.addView(status);

        TextView viewportLabel = new TextView(this);
        viewportLabel.setText("3D VIEWPORT  •  NATIVE VULKAN");
        viewportLabel.setTextColor(0xFF7DE0C3);
        viewportLabel.setTypeface(Typeface.DEFAULT_BOLD);
        root.addView(viewportLabel);

        viewport = new SurfaceView(this);
        viewport.setBackground(null);
        viewport.getHolder().addCallback(this);
        int viewportHeight = (int) (180 * getResources().getDisplayMetrics().density);
        root.addView(viewport, new LinearLayout.LayoutParams(-1, viewportHeight));

        TextView treeLabel = new TextView(this);
        treeLabel.setText("SCENE TREE  •  Main.IxoScene");
        treeLabel.setTextColor(0xFF9CC7FF);
        treeLabel.setTypeface(Typeface.DEFAULT_BOLD);
        root.addView(treeLabel);
        sceneTree = new TextView(this);
        sceneTree.setTextColor(0xFFE5ECF7);
        sceneTree.setTextSize(12);
        sceneTree.setPadding(8, 4, 8, 4);
        root.addView(sceneTree);
        refreshSceneTree();

        TextView path = new TextView(this);
        path.setText("res://Scripts/Player.Ixo.scripting");
        path.setTextColor(0xFF7DE0C3);
        root.addView(path);

        LinearLayout nodes = new LinearLayout(this);
        nodes.setOrientation(LinearLayout.HORIZONTAL);
        for (String node : new String[]{"Node3D", "Camera3D", "Mesh3D", "Light3D"}) {
            Button b = new Button(this);
            b.setText(node);
            b.setTextSize(10);
            b.setOnClickListener(v -> {
                addSceneNode(node);
            });
            nodes.addView(b, new LinearLayout.LayoutParams(0, -2, 1));
        }
        root.addView(nodes);

        editor = new EditText(this);
        editor.setGravity(Gravity.TOP | Gravity.START);
        editor.setTypeface(Typeface.MONOSPACE);
        editor.setTextSize(13);
        editor.setTextColor(0xFFE5ECF7);
        editor.setHintTextColor(0xFF7C879A);
        editor.setHint("Write Lua-style Ixo scripts here...");
        editor.setBackgroundColor(0xFF1B2230);
        editor.setPadding(12, 12, 12, 12);
        try { editor.setText(new String(java.nio.file.Files.readAllBytes(scriptFile.toPath()), StandardCharsets.UTF_8)); }
        catch (Exception ignored) { }
        ScrollView scroll = new ScrollView(this);
        scroll.addView(editor);
        root.addView(scroll, new LinearLayout.LayoutParams(-1, 0, 1));

        Button save = new Button(this);
        save.setText("SAVE SCRIPT");
        save.setOnClickListener(v -> saveScript());
        root.addView(save);

        TextView footer = new TextView(this);
        footer.setText("Scene: Main.IxoScene  •  Vulkan swapchain viewport");
        footer.setTextColor(0xFF8E9CB2);
        footer.setPadding(0, 10, 0, 0);
        root.addView(footer);
        setContentView(root);
    }

    @Override public void surfaceCreated(SurfaceHolder holder) {
        rendererStarted = nativeSurfaceCreated(holder.getSurface(), getAssets());
        status.setText("Project: IxoProject\n" + (rendererStarted
            ? "Vulkan device + Android surface ready; creating swapchain…"
            : "Vulkan surface initialization failed. Check native logcat."));
    }

    @Override public void surfaceChanged(SurfaceHolder holder, int format, int width, int height) {
        if (!rendererStarted || width <= 0 || height <= 0) return;
        nativeSurfaceChanged(width, height);
        status.setText("Project: IxoProject\nVulkan swapchain viewport: " + width + " × " + height);
    }

    @Override public void surfaceDestroyed(SurfaceHolder holder) {
        nativeSurfaceDestroyed();
        rendererStarted = false;
        status.setText("Project: IxoProject\nVulkan surface released.");
    }

    @Override protected void onDestroy() {
        if (rendererStarted) {
            nativeSurfaceDestroyed();
            rendererStarted = false;
        }
        super.onDestroy();
    }


    private JSONObject defaultScene() {
        JSONObject scene = new JSONObject();
        try {
            scene.put("format", "ixo-scene");
            scene.put("version", 1);
            scene.put("name", "Main");
            JSONArray nodes = new JSONArray();
            nodes.put(new JSONObject().put("name", "World").put("type", "Node3D").put("parent", JSONObject.NULL));
            nodes.put(new JSONObject().put("name", "Camera3D").put("type", "Camera3D").put("parent", "World").put("current", true));
            nodes.put(new JSONObject().put("name", "DirectionalLight3D").put("type", "DirectionalLight3D").put("parent", "World"));
            nodes.put(new JSONObject().put("name", "Ground").put("type", "MeshInstance3D").put("parent", "World").put("mesh", "PlaneMesh"));
            scene.put("nodes", nodes);
        } catch (Exception ignored) { }
        return scene;
    }

    private JSONObject loadScene() {
        try {
            byte[] bytes = java.nio.file.Files.readAllBytes(sceneFile.toPath());
            return new JSONObject(new String(bytes, StandardCharsets.UTF_8));
        } catch (Exception e) {
            return defaultScene();
        }
    }

    private void saveScene(JSONObject scene) {
        try (FileOutputStream out = new FileOutputStream(sceneFile)) {
            out.write((scene.toString(2) + "\n").getBytes(StandardCharsets.UTF_8));
        } catch (Exception e) {
            Toast.makeText(this, "Scene save failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private void addSceneNode(String buttonName) {
        String type;
        switch (buttonName) {
            case "Camera3D": type = "Camera3D"; break;
            case "Mesh3D": type = "MeshInstance3D"; break;
            case "Light3D": type = "DirectionalLight3D"; break;
            default: type = "Node3D"; break;
        }
        JSONObject scene = loadScene();
        try {
            JSONArray nodes = scene.optJSONArray("nodes");
            if (nodes == null) nodes = new JSONArray();
            String base = type;
            int suffix = 2;
            String name = base;
            boolean duplicate = true;
            while (duplicate) {
                duplicate = false;
                for (int i = 0; i < nodes.length(); i++) {
                    if (name.equals(nodes.getJSONObject(i).optString("name"))) {
                        duplicate = true;
                        name = base + suffix++;
                        break;
                    }
                }
            }
            JSONObject node = new JSONObject();
            node.put("name", name);
            node.put("type", type);
            node.put("parent", "World");
            if ("Camera3D".equals(type)) node.put("current", false);
            if ("MeshInstance3D".equals(type)) node.put("mesh", "BoxMesh");
            nodes.put(node);
            scene.put("nodes", nodes);
            saveScene(scene);
            refreshSceneTree();
            Toast.makeText(this, "Added " + name + " to Main.IxoScene", Toast.LENGTH_SHORT).show();
        } catch (Exception e) {
            Toast.makeText(this, "Node creation failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    private void refreshSceneTree() {
        if (sceneTree == null) return;
        JSONObject scene = loadScene();
        JSONArray nodes = scene.optJSONArray("nodes");
        StringBuilder tree = new StringBuilder();
        if (nodes != null) {
            for (int i = 0; i < nodes.length(); i++) {
                JSONObject node = nodes.optJSONObject(i);
                if (node != null) tree.append("  └ ").append(node.optString("name", "Unnamed"))
                    .append("  [").append(node.optString("type", "Node3D")).append("]\n");
            }
        }
        sceneTree.setText(tree.toString());
    }

    private void saveScript() {
        try (FileOutputStream out = new FileOutputStream(scriptFile)) {
            out.write(editor.getText().toString().getBytes(StandardCharsets.UTF_8));
            Toast.makeText(this, "Saved to " + scriptFile.getAbsolutePath(), Toast.LENGTH_LONG).show();
            status.setText("Project: IxoProject\n" + (rendererStarted ? "Vulkan viewport active" : nativeRendererStatus()) + "\nScript saved.");
        } catch (Exception e) {
            Toast.makeText(this, "Save failed: " + e.getMessage(), Toast.LENGTH_LONG).show();
        }
    }
}
