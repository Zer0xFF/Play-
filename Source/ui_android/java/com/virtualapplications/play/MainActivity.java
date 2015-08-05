package com.virtualapplications.play;

import android.app.*;
import android.app.ProgressDialog;
import android.content.*;
import android.content.pm.*;
import android.content.res.Configuration;
import android.content.res.TypedArray;
import android.graphics.Color;
import android.graphics.LinearGradient;
import android.graphics.Shader;
import android.graphics.drawable.Drawable;
import android.graphics.drawable.GradientDrawable;
import android.graphics.drawable.LayerDrawable;
import android.graphics.drawable.PaintDrawable;
import android.graphics.drawable.ShapeDrawable;
import android.graphics.drawable.shapes.RectShape;
import android.os.*;
import android.support.v7.app.ActionBarActivity;
import android.support.v7.widget.Toolbar;
import android.util.*;
import android.view.*;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.*;
import android.widget.TableLayout;
import android.widget.TableRow;
import android.widget.TextView;
import android.support.v4.widget.DrawerLayout;
import android.support.v4.app.ActionBarDrawerToggle;
import java.io.*;
import java.lang.reflect.InvocationTargetException;
import java.text.*;
import java.util.*;
import java.util.zip.*;
import org.apache.commons.lang3.StringUtils;
import com.android.util.FileUtils;
import android.graphics.Point;

import com.virtualapplications.play.database.GameInfo;
import com.virtualapplications.play.database.SqliteHelper.Games;

public class MainActivity extends ActionBarActivity implements NavigationDrawerFragment.NavigationDrawerCallbacks
{	



	private int currentOrientation;
	protected NavigationDrawerFragment mNavigationDrawerFragment;
	static Activity mActivity;
	static SharedPreferences _preferences;
	static final String PREFERENCE_CURRENT_DIRECTORY = "CurrentDirectory";

	@Override 
	protected void onCreate(Bundle savedInstanceState) 
	{
		super.onCreate(savedInstanceState);
		//Log.w(Constants.TAG, "MainActivity - onCreate");
		
		currentOrientation = getResources().getConfiguration().orientation;


		setContentView(R.layout.main);

		Toolbar toolbar = (Toolbar) findViewById(R.id.my_awesome_toolbar);
		setSupportActionBar(toolbar);
		toolbar.bringToFront();


		mNavigationDrawerFragment = (NavigationDrawerFragment)
				getFragmentManager().findFragmentById(R.id.navigation_drawer);

		// Set up the drawer.
		mNavigationDrawerFragment.setUp(
				R.id.navigation_drawer,
				(DrawerLayout) findViewById(R.id.drawer_layout));

		TypedArray a = getTheme().obtainStyledAttributes(new int[]{R.attr.colorPrimaryDark});
		int attributeResourceId = a.getColor(0, 0);
		a.recycle();
		findViewById(R.id.navigation_drawer).setBackgroundColor(Color.parseColor(
				("#" + Integer.toHexString(attributeResourceId)).replace("#ff", "#8e")
		));


		mActivity = MainActivity.this;
	}

	@Override 
	protected void onPostCreate(Bundle savedInstanceState) {
		super.onPostCreate(savedInstanceState);


		NativeInterop.setFilesDirPath(Environment.getExternalStorageDirectory().getAbsolutePath());

		_preferences = getSharedPreferences("prefs", MODE_PRIVATE);
		EmulatorActivity.RegisterPreferences();

		if(!NativeInterop.isVirtualMachineCreated())
		{
			NativeInterop.createVirtualMachine();
		}

		adjustUI();


		FragmentTransaction ft = getFragmentManager().beginTransaction();
		ft.replace(R.id.content_frame, new All_Games_Fragment().newInstance(null, null));
		ft.commit();

	}

	int getStatusBarHeight() {
		int result = 0;
		int resourceId = getResources().getIdentifier("status_bar_height", "dimen", "android");
		if (resourceId > 0) {
			result = getResources().getDimensionPixelSize(resourceId);
		}
		return result;
	}

	void adjustUI() {
		//this sets toolbar margin, but in effect moving the DrawerLayout
		int statusBarHeight = getStatusBarHeight();

		View toolbar = findViewById(R.id.my_awesome_toolbar);
		final FrameLayout content = (FrameLayout) findViewById(R.id.content_frame);
		
		ViewGroup.MarginLayoutParams dlp = (ViewGroup.MarginLayoutParams) content.getLayoutParams();
		dlp.topMargin = statusBarHeight;
		content.setLayoutParams(dlp);

		int[] colors = new int[2];// you can increase array size to add more colors to gradient.
		TypedArray a = getTheme().obtainStyledAttributes(new int[]{R.attr.colorPrimary});
		int attributeResourceId = a.getColor(0, 0);
		a.recycle();
		float[] hsv = new float[3];
		Color.colorToHSV(attributeResourceId, hsv);
		hsv[2] *= 0.8f;// make it darker
		colors[0] = Color.HSVToColor(hsv);
		/*
		using this will blend the top of the gradient with actionbar (aka using the same color)
		colors[0] = Color.parseColor("#" + Integer.toHexString(attributeResourceId)
		 */
		colors[1] = Color.rgb(20,20,20);
		GradientDrawable gradientbg = new GradientDrawable(GradientDrawable.Orientation.TOP_BOTTOM, colors);
		content.setBackground(gradientbg);

		ViewGroup.MarginLayoutParams mlp = (ViewGroup.MarginLayoutParams) toolbar.getLayoutParams();
		mlp.bottomMargin = - statusBarHeight;
		toolbar.setLayoutParams(mlp);
		View navigation_drawer = findViewById(R.id.navigation_drawer);
		ViewGroup.MarginLayoutParams mlp2 = (ViewGroup.MarginLayoutParams) navigation_drawer.getLayoutParams();
		mlp2.topMargin = statusBarHeight;
		navigation_drawer.setLayoutParams(mlp2);

		Point p = getNavigationBarSize(this);
		/*
		This will take account of nav bar to right/bottom
		Not sure if there is a way to detect left/top? thus always pad right/bottom for now
		*/
		if (p.x != 0){
			View relative_layout = findViewById(R.id.relative_layout);
			relative_layout.setPadding(
					relative_layout.getPaddingLeft(), 
					relative_layout.getPaddingTop(), 
					relative_layout.getPaddingRight() + p.x, 
					relative_layout.getPaddingBottom());
		} else if (p.y != 0){
			navigation_drawer.setPadding(
				navigation_drawer.getPaddingLeft(), 
				navigation_drawer.getPaddingTop(), 
				navigation_drawer.getPaddingRight(), 
				navigation_drawer.getPaddingBottom() + p.y);

		/*View game_scroller = findViewById(R.id.game_grid);
			game_scroller.setPadding(
			game_scroller.getPaddingLeft(), 
			game_scroller.getPaddingTop(), 
			game_scroller.getPaddingRight(), 
			game_scroller.getPaddingBottom() + p.y);
		*/
		}
	}

	static Point getNavigationBarSize(Context context) {
		Point appUsableSize = getAppUsableScreenSize(context);
		Point realScreenSize = getRealScreenSize(context);
		return new Point(realScreenSize.x - appUsableSize.x, realScreenSize.y - appUsableSize.y);
	}

	static Point getAppUsableScreenSize(Context context) {
		WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
		Display display = windowManager.getDefaultDisplay();
		Point size = new Point();
		display.getSize(size);
		return size;
	}

	static Point getRealScreenSize(Context context) {
		WindowManager windowManager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
		Display display = windowManager.getDefaultDisplay();
		Point size = new Point();

		if (Build.VERSION.SDK_INT >= 17) {
		display.getRealSize(size);
		} else if (Build.VERSION.SDK_INT >= 14) {
		try {
			size.x = (Integer) Display.class.getMethod("getRawWidth").invoke(display);
			size.y = (Integer) Display.class.getMethod("getRawHeight").invoke(display);
		} catch (IllegalAccessException e) {} catch (InvocationTargetException e) {} catch (NoSuchMethodException e) {}
		}

		return size;
	}

	private void displaySettingsActivity() {
		Intent intent = new Intent(getApplicationContext(), SettingsActivity.class);
		startActivity(intent);
	}

	private void displayAboutDialog() {
		long buildDate = getBuildDate(this);
		String buildDateString = new SimpleDateFormat("yyyy/MM/dd", Locale.getDefault()).format(buildDate);
		String aboutMessage = String.format("Build Date: %s", buildDateString);
		displaySimpleMessage("About Play!", aboutMessage, this);
	}

	static void displaySimpleMessage(String title, String message, Context context) {
		AlertDialog.Builder builder = new AlertDialog.Builder(context);

		builder.setTitle(title);
		builder.setMessage(message);

		builder.setPositiveButton("OK",
				new DialogInterface.OnClickListener()
				{
					@Override
					public void onClick(DialogInterface dialog, int id)
					{

					}
				}
		);

		AlertDialog dialog = builder.create();
		dialog.show();
	}

	static long getBuildDate(Context context)  {
		try
		{
			ApplicationInfo ai = context.getPackageManager().getApplicationInfo(context.getPackageName(), 0);
			ZipFile zf = new ZipFile(ai.sourceDir);
			ZipEntry ze = zf.getEntry("classes.dex");
			long time = ze.getTime();
			return time;
		}
		catch (Exception e)
		{

		}
		return 0;
	}

	public void restoreActionBar() {
		android.support.v7.app.ActionBar actionBar = getSupportActionBar();
		actionBar.setNavigationMode(ActionBar.NAVIGATION_MODE_STANDARD);
		actionBar.setDisplayShowTitleEnabled(true);
		actionBar.setTitle(R.string.app_name);
		actionBar.setSubtitle(R.string.menu_title_shut);
	}

	public boolean onCreateOptionsMenu(Menu menu) {
		if (!mNavigationDrawerFragment.isDrawerOpen()) {
			// Only show items in the action bar relevant to this screen
			// if the drawer is not showing. Otherwise, let the drawer
			// decide what to show in the action bar.
			//getMenuInflater().inflate(R.menu.main, menu);
			restoreActionBar();
			return true;
		}
		return super.onCreateOptionsMenu(menu);
	}

	@Override
	public void onBackPressed() {
		if (mNavigationDrawerFragment.mDrawerLayout != null && mNavigationDrawerFragment.isDrawerOpen()) {
			mNavigationDrawerFragment.mDrawerLayout.closeDrawer(NavigationDrawerFragment.mFragmentContainerView);
			return;
		}
		super.onBackPressed();
		finish();
	}

	@Override
	public void onConfigurationChanged(Configuration newConfig) {
		super.onConfigurationChanged(newConfig);
		mNavigationDrawerFragment.onConfigurationChanged(newConfig);

	}

	@Override
	public void onNavigationDrawerItemSelected(int position) {
		//This will only manages the top stack, which is currently empty
	}

	@Override
	public void onNavigationDrawerBottomItemSelected(int position) {
		switch (position) {
			case 0:
				displaySettingsActivity();
				break;
			case 1:
				displayAboutDialog();
				break;

		}
	}

	private void clearCoverCache() {
		File dir = new File(getExternalFilesDir(null), "covers");
		for (File file : dir.listFiles()) {
			if (!file.isDirectory()) {
				file.delete();
			}
		}
	}

	static void clearCache() {
		((MainActivity) mActivity).clearCoverCache();
	}

	static boolean IsLoadableExecutableFileName(String fileName) {
		return fileName.toLowerCase().endsWith(".elf");
	}

	private static boolean IsLoadableDiskImageFileName(String fileName) {

		return
				fileName.toLowerCase().endsWith(".iso") ||
						fileName.toLowerCase().endsWith(".bin") ||
						fileName.toLowerCase().endsWith(".cso") ||
						fileName.toLowerCase().endsWith(".isz");
	}

	public static void launchDisk(File game, Context context) {
		try
		{
			if(IsLoadableExecutableFileName(game.getPath()))
			{
				NativeInterop.loadElf(game.getPath());
			}
			else
			{
				NativeInterop.bootDiskImage(game.getPath());
			}
		}
		catch(Exception ex)
		{
			MainActivity.displaySimpleMessage("Error", ex.getMessage(), context);
			return;
		}
		//TODO: Catch errors that might happen while loading files
		Intent intent = new Intent(context, EmulatorActivity.class);
		context.startActivity(intent);
	}


}
