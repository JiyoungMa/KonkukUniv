package com.example.whatsappclone;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import android.database.Cursor;
import android.os.Bundle;
import android.provider.ContactsContract;
import android.telephony.TelephonyManager;
import android.widget.LinearLayout;

import com.example.whatsappclone.User.UserListAdapter;
import com.example.whatsappclone.User.UserObject;
import com.example.whatsappclone.Utils.CountryToPhonePrefix;
import com.google.firebase.database.DataSnapshot;
import com.google.firebase.database.DatabaseError;
import com.google.firebase.database.DatabaseReference;
import com.google.firebase.database.FirebaseDatabase;
import com.google.firebase.database.Query;
import com.google.firebase.database.ValueEventListener;

import java.util.ArrayList;

public class FindUserAcitivity extends AppCompatActivity {

    private RecyclerView mUserList;
    private RecyclerView.Adapter mUserListAdaptter;
    private RecyclerView.LayoutManager mUserListLayoutManager;

    ArrayList<UserObject> userList,contactList;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_find_user);

        contactList = new ArrayList<>();
        userList = new ArrayList<>();

        initializeRecyclerView();
        getContactList();
    }
    private void getContactList(){

        String ISOPrefix = getCountryISO();

        Cursor phones = getContentResolver().query(ContactsContract.CommonDataKinds.Phone.CONTENT_URI,null,null,null,null);
        //Cursor를 하나씩 움직이기 위한 반복문
        while(phones.moveToNext()){
            String name = phones.getString(phones.getColumnIndex(ContactsContract.CommonDataKinds.Phone.DISPLAY_NAME));
            String phone = phones.getString(phones.getColumnIndex(ContactsContract.CommonDataKinds.Phone.NUMBER));

            phone = phone.replace(" ","");
            phone = phone.replace("-","");
            phone = phone.replace("(","");
            phone = phone.replace(")","");

            if(!String.valueOf(phone.charAt(0)).equals("+")){ // Country indicator이 없다면
                phone = ISOPrefix+phone;
            }

            UserObject mContact = new UserObject("", name,phone);
            contactList.add(mContact);
            getUserDetails(mContact);//연락처 중, DB에 등록 되어있는 것만 가져오기 (해당 서비스를 같이 사용중인 사람의 정보만 가져오기
        }
    }

    private void getUserDetails(final UserObject mContact) {
        DatabaseReference mUserDB = FirebaseDatabase.getInstance().getReference().child("user");
        Query query = mUserDB.orderByChild("phone").equalTo(mContact.getPhone());
        query.addListenerForSingleValueEvent(new ValueEventListener() {
            @Override
            public void onDataChange(@NonNull DataSnapshot dataSnapshot) {
                if(dataSnapshot.exists()){
                    String phone = "",
                            name = "";

                    for(DataSnapshot childSnapshot : dataSnapshot.getChildren()){
                        if(childSnapshot.child("phone").getValue() != null){
                            phone = childSnapshot.child("phone").getValue().toString();
                        }
                        if(childSnapshot.child("name").getValue() != null){
                            name = childSnapshot.child("name").getValue().toString();
                        }

                        UserObject mUser = new UserObject(childSnapshot.getKey(),name,phone);
                        //Getting Name of the User
                        //현재의 phone number과 연락처의 phone number 비교 후, 같으면, 연락처의 이름을 가져옴
                        if(name.equals(phone)){
                            for(UserObject mContactIterator : contactList){
                                if(mContactIterator.getPhone().equals(mUser.getPhone())){
                                    mUser.setName(mContactIterator.getName());
                                }
                            }
                        }

                        userList.add(mUser);
                        mUserListAdaptter.notifyDataSetChanged();;
                        return;
                    }
                }
            }

            @Override
            public void onCancelled(@NonNull DatabaseError databaseError) {            }
        });
    }

    private String getCountryISO(){ //Country indicator이 없다면, 해당 네트워크를 통해, country indicator를 제공
        String iso = null;

        TelephonyManager telephonyManager = (TelephonyManager)getApplicationContext().getSystemService(getApplicationContext().TELEPHONY_SERVICE);
        if(telephonyManager.getNetworkCountryIso() != null){
            if(!telephonyManager.getNetworkCountryIso().toString().equals("")){
                iso = telephonyManager.getNetworkCountryIso().toString();
            }
        }

        return CountryToPhonePrefix.getPhone(iso);
    }


    private void initializeRecyclerView() {
        mUserList = findViewById(R.id.userList);
        mUserList.setNestedScrollingEnabled(false); //스크롤안보이게 하기
        mUserList.setHasFixedSize(false);
        mUserListLayoutManager = new LinearLayoutManager(getApplicationContext(),LinearLayout.VERTICAL,false); //UserList에 뭔가 특별한 일을 해주고 싶을 때 사용함
        mUserList.setLayoutManager(mUserListLayoutManager); //Manager 연결해주기
        mUserListAdaptter = new UserListAdapter(userList);
        mUserList.setAdapter(mUserListAdaptter);

    }
}
