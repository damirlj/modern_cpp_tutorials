package com.example.practice_kotlin.design_patterns.builder

import org.junit.Test

class TestBuilderPattern {

    @Test
    fun testBuilderPattern() {
        var builder = A.Companion.Builder()
        val a = builder.setId(11).setName("Alex").build()
        println(a)
        println(A.Companion.Builder(a).setName("Divna").build())

        builder = A.Companion.Builder()
        println(builder.setName("Damir").build())
        println(A.update(a, builder.build()))
    }

    @Test
    fun testBuilderPatternDSL() {

        val car = CarConfiguration.build(123, "Audi", Engine.DIESEL) {
            camera = Camera.FRONT
            smartphone = Smartphone.CarPlay
        }

        println(car)
        println(car.smartphone)

        println(CarConfiguration.build(car, CarConfiguration.build(car) {
            adas = ADAS.AdaptiveControl
        }))
        println(CarConfiguration.build(car, CarConfiguration.build(124, "VW", Engine.ELECTRIC) {
            camera = Camera.REAR
            adas = ADAS.Copilot
        }))
    }

    data class User2 (val id: Int, val name: String, var email : String? = null)
    @Test
    fun testBuilderPatternWithApplyOnly() {
        var car = CarConfiguration2(123, "Audi", Engine.DIESEL)
        println(car)
        car = CarConfiguration2.build(car) {
            camera = Camera.REAR
            adas = ADAS.Copilot
        }
        println(car)

        var user = User2(1, "Alex")
        println(user)

        user = user.apply{
            email = "alexlj@yahoo.com"
        }
        println(user)
    }
}