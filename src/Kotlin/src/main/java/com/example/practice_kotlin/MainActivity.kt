package com.example.practice_kotlin

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.Surface
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Modifier
import androidx.compose.ui.tooling.preview.Preview
import com.example.practice_kotlin.helpers.func_elapsed
import com.example.practice_kotlin.ui.theme.Practice_KotlinTheme


class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContent {
            Practice_KotlinTheme {
                // A surface container using the 'background' color from the theme
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = MaterialTheme.colorScheme.background
                ) {
                    Greeting()
                }
            }
        }
    }
}

private fun test() {
    for (i in 1..5) {
        Thread.sleep(10)
    }
}

@Composable
fun Greeting(modifier: Modifier = Modifier) {
   val (_, elapsedTime) = func_elapsed(::test)
    Text(
        text = "Elapsed time: $elapsedTime",
        modifier = modifier
    )

}

@Preview(showBackground = true)
@Composable
fun GreetingPreview() {

    Practice_KotlinTheme {
        Greeting()
    }
}