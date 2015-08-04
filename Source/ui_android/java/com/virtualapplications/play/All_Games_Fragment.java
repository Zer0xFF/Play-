package com.virtualapplications.play;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.app.UiModeManager;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.ApplicationInfo;
import android.content.res.Configuration;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.app.Fragment;
import android.os.Environment;
import android.util.DisplayMetrics;
import android.view.Gravity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;

import com.android.util.FileUtils;
import com.virtualapplications.play.database.GameInfo;
import com.virtualapplications.play.database.SqliteHelper;

import org.apache.commons.lang3.StringUtils;

import java.io.File;
import java.io.FilenameFilter;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashSet;
import java.util.Iterator;
import java.util.List;
import java.util.Locale;
import java.util.zip.ZipEntry;
import java.util.zip.ZipFile;

import android.content.Context.*;

/**
 * A simple {@link Fragment} subclass.
 * Activities that contain this fragment must implement the
 * Use the {@link All_Games_Fragment#newInstance} factory method to
 * create an instance of this fragment.
 */
public class All_Games_Fragment extends Fragment {
    // TODO: Rename parameter arguments, choose names that match
    // the fragment initialization parameters, e.g. ARG_ITEM_NUMBER
    private static final String ARG_PARAM1 = "param1";
    private static final String ARG_PARAM2 = "param2";

    // TODO: Rename and change types of parameters
    private String mParam1;
    private String mParam2;
    private TableLayout gameListing;
    private static boolean isConfigured = false;
    private int numColumn = 0;
    private float localScale;
    private GameInfo gameInfo;

    /**
     * Use this factory method to create a new instance of
     * this fragment using the provided parameters.
     *
     * @param param1 Parameter 1.
     * @param param2 Parameter 2.
     * @return A new instance of fragment all_games.
     */
    // TODO: Rename and change types and number of parameters
    public static All_Games_Fragment newInstance(String param1, String param2) {
        All_Games_Fragment fragment = new All_Games_Fragment();
        Bundle args = new Bundle();
        args.putString(ARG_PARAM1, param1);
        args.putString(ARG_PARAM2, param2);
        fragment.setArguments(args);
        return fragment;
    }

    public All_Games_Fragment() {
        // Required empty public constructor
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getArguments() != null) {
            mParam1 = getArguments().getString(ARG_PARAM1);
            mParam2 = getArguments().getString(ARG_PARAM2);
        }
    }

    @Override
    public View onCreateView(LayoutInflater inflater, ViewGroup container,
                             Bundle savedInstanceState) {
        // Inflate the layout for this fragment


        return inflater.inflate(R.layout.fragment_all_games, container, false);
    }

    @Override
    public void onStart(){
        super.onStart();
        gameInfo = new GameInfo(this.getActivity());
        getActivity().getContentResolver().call(SqliteHelper.Games.GAMES_URI, "importDb", null, null);
        prepareFileListView();
    }

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
    }

    @Override
    public void onDetach() {
        super.onDetach();
    }


    private String getCurrentDirectory() {

        return MainActivity._preferences.getString(MainActivity.PREFERENCE_CURRENT_DIRECTORY, Environment.getExternalStorageDirectory().getAbsolutePath());
    }

    private void setCurrentDirectory(String currentDirectory) {
        SharedPreferences.Editor preferencesEditor = MainActivity._preferences.edit();
        preferencesEditor.putString(MainActivity.PREFERENCE_CURRENT_DIRECTORY, currentDirectory);
        preferencesEditor.commit();
    }

    public static HashSet<String> getExternalMounts() {
        final HashSet<String> out = new HashSet<String>();
        String reg = "(?i).*vold.*(vfat|ntfs|exfat|fat32|ext3|ext4|fuse).*rw.*";
        String s = "";
        try {
            final java.lang.Process process = new ProcessBuilder().command("mount")
                    .redirectErrorStream(true).start();
            process.waitFor();
            final InputStream is = process.getInputStream();
            final byte[] buffer = new byte[1024];
            while (is.read(buffer) != -1) {
                s = s + new String(buffer);
            }
            is.close();
        } catch (final Exception e) {

        }

        final String[] lines = s.split("\n");
        for (String line : lines) {
            if (StringUtils.containsIgnoreCase(line, "secure"))
                continue;
            if (StringUtils.containsIgnoreCase(line, "asec"))
                continue;
            if (line.matches(reg)) {
                String[] parts = line.split(" ");
                for (String part : parts) {
                    if (part.startsWith("/"))
                        if (!StringUtils.containsIgnoreCase(part, "vold"))
                            out.add(part);
                }
            }
        }
        return out;
    }


    private static void clearCurrentDirectory() {
        SharedPreferences.Editor preferencesEditor = MainActivity._preferences.edit();
        preferencesEditor.remove(MainActivity.PREFERENCE_CURRENT_DIRECTORY);
        preferencesEditor.commit();
    }

    public static void resetDirectory() {
        All_Games_Fragment.clearCurrentDirectory();
        All_Games_Fragment.isConfigured = false;
        //All_Games_Fragment.prepareFileListView();
    }

    private final class ImageFinder extends AsyncTask<String, Integer, List<File>> {

        private int array;
        private ProgressDialog progDialog;

        public ImageFinder(int arrayType) {
            this.array = arrayType;
        }

        private List<File> getFileList(String path) {
            File storage = new File(path);

            String[] mediaTypes = getResources().getStringArray(array);
            FilenameFilter[] filter = new FilenameFilter[mediaTypes.length];

            int i = 0;
            for (final String type : mediaTypes) {
                filter[i] = new FilenameFilter() {

                    public boolean accept(File dir, String name) {
                        if (dir.getName().startsWith(".") || name.startsWith(".")) {
                            return false;
                        } else if (StringUtils.endsWithIgnoreCase(name, "." + type)) {
                            File disk = new File(dir, name);
                            String serial = gameInfo.getSerial(disk);
                            return MainActivity.IsLoadableExecutableFileName(disk.getPath()) ||
                                    (serial != null && !serial.equals(""));
                        } else {
                            return false;
                        }
                    }

                };
                i++;
            }
            FileUtils fileUtils = new FileUtils();
            Collection<File> files = fileUtils.listFiles(storage, filter, -1);
            return (List<File>) files;
        }

        private View createListItem(final File game, final int index) {

            if (!isConfigured) {

                final View childview = getActivity().getLayoutInflater().inflate(
                        R.layout.file_list_item, null, false);

                ((TextView) childview.findViewById(R.id.game_text)).setText(game.getName());

                childview.findViewById(R.id.childview).setOnClickListener(new View.OnClickListener() {
                    public void onClick(View view) {
                        setCurrentDirectory(game.getPath().substring(0,
                                game.getPath().lastIndexOf(File.separator)));
                        isConfigured = true;
                        prepareFileListView();
                        return;
                    }
                });

                return childview;
            }

            final View childview = getActivity().getLayoutInflater().inflate(
                    R.layout.game_list_item, null, false);

            ((TextView) childview.findViewById(R.id.game_text)).setText(game.getName());

            final String[] gameStats = gameInfo.getGameInfo(game, childview);

            if (gameStats != null) {
                childview.findViewById(R.id.childview).setOnLongClickListener(
                        gameInfo.configureLongClick(gameStats[1], gameStats[2], game));

                if (!gameStats[3].equals("404")) {
                    gameInfo.getImage(gameStats[0], childview, gameStats[3]);
                    ((TextView) childview.findViewById(R.id.game_text)).setVisibility(View.GONE);
                }
            }

            childview.findViewById(R.id.childview).setOnClickListener(new View.OnClickListener() {
                public void onClick(View view) {
                    launchDisk(game);
                    return;
                }
            });
            return childview;
        }

        protected void onPreExecute() {
            progDialog = ProgressDialog.show(getActivity(),
                    getString(R.string.search_games),
                    getString(R.string.search_games_msg), true);
        }

        @Override
        protected List<File> doInBackground(String... paths) {

            final String root_path = paths[0];
            ArrayList<File> files = new ArrayList<File>();
            files.addAll(getFileList(root_path));

            if (!isConfigured) {
                HashSet<String> extStorage = getExternalMounts();
                if (extStorage != null && !extStorage.isEmpty()) {
                    for (Iterator<String> sd = extStorage.iterator(); sd.hasNext();) {
                        String sdCardPath = sd.next().replace("mnt/media_rw", "storage");
                        if (!sdCardPath.equals(root_path)) {
                            if (new File(sdCardPath).canRead()) {
                                files.addAll(getFileList(sdCardPath));
                            }
                        }
                    }
                }
            }
            return (List<File>) files;
        }

        @Override
        protected void onPostExecute(List<File> images) {
            if (progDialog != null && progDialog.isShowing()) {
                progDialog.dismiss();
            }
            if (images != null && !images.isEmpty()) {
                // Create the list of acceptable images

                Collections.sort(images);

                TableRow game_row = new TableRow(getActivity());
                if (isConfigured) {
                    game_row.setGravity(Gravity.CENTER);
                }
                int pad = (int) (10 * localScale + 0.5f);
                game_row.setPadding(0, 0, 0, pad);

                if (!isConfigured) {
                    TableRow.LayoutParams params = new TableRow.LayoutParams(
                            TableRow.LayoutParams.MATCH_PARENT,
                            TableRow.LayoutParams.WRAP_CONTENT);
                    params.gravity = Gravity.CENTER_VERTICAL;

                    for (int i = 0; i < images.size(); i++)
                    {
                        game_row.addView(createListItem(images.get(i), i));
                        gameListing.addView(game_row, params);
                        game_row = new TableRow(getActivity());
                        game_row.setPadding(0, 0, 0, pad);
                    }
                } else {
                    TableRow.LayoutParams params = new TableRow.LayoutParams(
                            TableRow.LayoutParams.WRAP_CONTENT,
                            TableRow.LayoutParams.WRAP_CONTENT);
                    params.gravity = Gravity.CENTER;

                    int column = 0;
                    for (int i = 0; i < images.size(); i++)
                    {
                        if (column == numColumn)
                        {
                            gameListing.addView(game_row, params);
                            column = 0;
                            game_row = new TableRow(getActivity());
                            game_row.setGravity(Gravity.CENTER);
                            game_row.setPadding(0, 0, 0, pad);
                        }
                        game_row.addView(createListItem(images.get(i), i));
                        column ++;
                    }
                    if (column != 0) {
                        gameListing.addView(game_row, params);
                    }
                }
                gameListing.invalidate();
            } else {
                // Display warning that no disks exist
            }
        }
    }

    public void launchDisk(File game) {
        setCurrentDirectory(game.getPath().substring(0, game.getPath().lastIndexOf(File.separator)));
        MainActivity.launchDisk(game, getActivity());
    }


    private boolean isAndroidTV(Context context) {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.ICE_CREAM_SANDWICH) {
            UiModeManager uiModeManager = (UiModeManager)
                    context.getSystemService(Context.UI_MODE_SERVICE);
            if (uiModeManager.getCurrentModeType() == Configuration.UI_MODE_TYPE_TELEVISION) {
                return true;
            }
        }
        return false;
    }

    private void prepareFileListView() {
        if (gameInfo == null) {
            gameInfo = new GameInfo(this.getActivity());
        }

        gameListing = (TableLayout) getActivity().findViewById(R.id.game_grid);
        if (gameListing != null) {
            gameListing.removeAllViews();
        }

        DisplayMetrics metrics = new DisplayMetrics();
        getActivity().getWindowManager().getDefaultDisplay().getMetrics(metrics);
        localScale = getResources().getDisplayMetrics().density;
        int screenWidth = (int) (metrics.widthPixels * localScale + 0.5f);
        int screenHeight = (int) (metrics.heightPixels * localScale + 0.5f);

        if (screenWidth > screenHeight) {
            numColumn = 3;
        } else {
            numColumn = 2;
        }

        if (isAndroidTV(getActivity())) {
            numColumn += 1;
        }

        String sdcard = getCurrentDirectory();
        if (!sdcard.equals(Environment.getExternalStorageDirectory().getAbsolutePath())) {
            isConfigured = true;
        }

        new ImageFinder(R.array.disks).execute(sdcard);

		/*if (!mDrawerLayout.isDrawerOpen(Gravity.LEFT)) {
			if (!isConfigured) {
				getActionBar().setTitle(getString(R.string.menu_title_look));
			} else {
				getActionBar().setTitle(getString(R.string.menu_title_shut));
			}
		}*/
    }
}
