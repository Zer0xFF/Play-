package com.virtualapplications.play;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.os.Environment;

import java.io.File;

public class ExternalEmulatorLauncher extends Activity {
    public ExternalEmulatorLauncher() {
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // TODO: This method is called when the BroadcastReceiver is receiving
        // an Intent broadcast.
        NativeInterop.setFilesDirPath(Environment.getExternalStorageDirectory().getAbsolutePath());

        EmulatorActivity.RegisterPreferences();

        if(!NativeInterop.isVirtualMachineCreated())
        {
            NativeInterop.createVirtualMachine();
        }
        Intent intent = getIntent();
        if (intent.getAction() != null) {
            if (intent.getAction().equals(Intent.ACTION_VIEW)) {
                MainActivity.launchDisk(new File(intent.getData().getPath()), this);
            }
        }
        finish();
    }
}
